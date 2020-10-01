#define STB_IMAGE_IMPLEMENTATION
#include "VulkanRenderer.h"

#include "Mesh.h"
#include "MeshModelData.h"
#include "MeshModel.h"

#include "Device.h"
#include "SwapChain.h"
#include "Image.h"
#include "ImageView.h"
#include "Instance.h"
#include "Buffer.h"
#include "CommandBuffer.h"
#include "Sampler.h"
#include "Surface.h"
#include "DescriptorPool.h"
#include "DescriptorSetLayout.h"
#include "DescriptorResourceReference.h"
#include "DescriptorSet.h"
#include "RenderTarget.h"
#include "Frame.h"
#include "Framebuffer.h"
#include "Texture.h"
#include "Queue.h"
#include "CommandBuffer.h"

VulkanRenderer::VulkanRenderer()
{
}

int VulkanRenderer::init(GLFWwindow* newWindow)
{
	mWindow = newWindow;

	try {
		setupThreadPool();
		createInstance();				// LEAVE
		createSurface();				// LEAVE
		createDevice();					// COMPLETE
		findDesiredQueueFamilies();
		createSwapchain();				// COMPLETE
		createPerFrameObjects();		// TODO : still need to create frame with descriptor sets, command buffers etc
		createRenderPass();				// Change to use rendertarget and subpass objects
		createDescriptorSetLayouts();
		createPushConstantRange();
		createGraphicsPipeline();
		createFramebuffers();
		createTextureSampler();
		//allocateDynamicBufferTransferSpace();
		createUniformBuffers();
		createDescriptorPools();
		//createDescriptorResourceReferences();
		createUniformDescriptorSets();
		createInputDescriptorSets();
		//createSynchronation();			// TODO : abstract to a pool
		createCamera(90.0f);

		//createTexture("default_checker.png");
		createTexture("default_black.png");

	}
	catch (const std::runtime_error& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;
}

void VulkanRenderer::updateModel(int modelId, glm::mat4 newModel)
{
	if (modelId >= modelList.size()) return;

	modelList[modelId].setModel(newModel);
}

void VulkanRenderer::createCamera(float FoVinDegrees)
{
	const VkExtent2D& extent = mSwapchain->extent();

	uboViewProjection.projection = glm::perspective(glm::radians(FoVinDegrees), (float)extent.width / (float)extent.height, 0.1f, 100.0f);
	uboViewProjection.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 20.0f), glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	uboViewProjection.projection[1][1] *= -1;
}

void VulkanRenderer::updateCameraView(mat4 newView)
{
	uboViewProjection.view = newView;
}

void VulkanRenderer::draw()
{
	auto& previousFrame = mFrames[activeFrameIndex];

	VkSemaphore imageAcquired = previousFrame->requestSemaphore();

	// TODO : get this semaphore form previous frame
	// Get next active frame
	VkResult result = mSwapchain->acquireNextImageIndex(imageAcquired, activeFrameIndex);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Could not acquire next swapchain image!");
	}

	auto& activeFrame = mFrames[activeFrameIndex];

	// Wait until idle then reset command pools + synchronisation objects
	activeFrame->wait();
	activeFrame->reset();

	// Request the required synchronisation objects
	VkSemaphore renderFinished = activeFrame->requestSemaphore();
	VkFence drawFence = activeFrame->requestFence();

	const Queue& queue = mDevice->queue(mGraphicsQueueFamily, 0);

	CommandBuffer& primaryCmdBuffer = activeFrame->requestCommandBuffer(queue, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	recordCommands(primaryCmdBuffer, activeFrameIndex);		// Only record commands once the image at imageIndex is available (not being used by the queue)

	updateUniformBuffers(activeFrameIndex);

	

	queue.submit(imageAcquired, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, renderFinished,
		primaryCmdBuffer, drawFence);

	queue.present(renderFinished, *mSwapchain, activeFrameIndex);


}

void VulkanRenderer::cleanup()
{
	// Wait until no actions being run on device before destroying
	vkDeviceWaitIdle(mDevice->logicalDevice());

	vkDestroyPipeline(mDevice->logicalDevice(), secondPipeline, nullptr);
	vkDestroyPipelineLayout(mDevice->logicalDevice(), secondPipelineLayout, nullptr);
	vkDestroyPipeline(mDevice->logicalDevice(), graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(mDevice->logicalDevice(), pipelineLayout, nullptr);
	vkDestroyRenderPass(mDevice->logicalDevice(), mRenderPass, nullptr);
}

VulkanRenderer::~VulkanRenderer()
{
	if (mDevice)
	{
		mDevice->waitIdle();
	}

	//vkDestroySurfaceKHR(mInstance->handle(), mSurface, nullptr);
	//vkDestroyInstance(mInstance, nullptr);
}

void VulkanRenderer::setupThreadPool()
{
	// TODO : revert
	//mThreadCount = std::thread::hardware_concurrency();
	mThreadCount = 1;
	mThreadPool.resize(mThreadCount);
}

void VulkanRenderer::createInstance()
{
	// Create vector to hold instance extensions - we must query for these values
	std::vector<const char*> requiredInstanceExtensions = {};

	uint32_t glfwExtensionCount = 0;						// GLFW may require multiple extensions
	const char** glfwExtensions;							// Extensions passed as array of cstrings, so need pointer (the array) to pinter (the cstring)

	// Get GLFW extensions
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// Add GLFW extensions to list of extensions
	for (size_t i = 0; i < glfwExtensionCount; ++i)
	{
		requiredInstanceExtensions.push_back(glfwExtensions[i]);
	}

	mInstance = std::make_unique<Instance>("Vulkan App", requiredInstanceExtensions);

}

// Create interface to the window
void VulkanRenderer::createSurface()
{
	VkSurfaceKHR surface;

	// Create Surface (creates a surface create info struct, runs the create surface function, returns result)
	VkResult result = glfwCreateWindowSurface(mInstance->handle(), mWindow, nullptr, &surface);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Surface!");
	}

	mSurface = std::make_unique<Surface>(*mInstance, surface);
}

// TODO : should not be creating the device extensions and features here
void VulkanRenderer::createDevice()
{
	// TEMPORARY
	std::vector<const char*> requiredExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkPhysicalDeviceFeatures requiredFeatures = {};
	requiredFeatures.samplerAnisotropy = VK_TRUE;

	mDevice = std::make_unique<Device>(*mInstance, mSurface->handle(), requiredExtensions, requiredFeatures);
}

// This should be redefined per application
// Get the indices of queues which will be used in the application
void VulkanRenderer::findDesiredQueueFamilies()
{
	mGraphicsQueueFamily = mDevice->getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
}

void VulkanRenderer::createSwapchain()
{
	VkExtent2D windowExtent;
	getWindowExtent(windowExtent);

	mSwapchain = std::make_unique<Swapchain>(*mDevice, windowExtent, mSurface->handle(), VK_PRESENT_MODE_MAILBOX_KHR, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
}

// Prepare rendertargets and frames
void VulkanRenderer::createPerFrameObjects()
{
	//auto& device = mSwapchain->device();
	auto& swapchainExtent = mSwapchain->extent();
	VkFormat swapchainFormat = mSwapchain->format();
	VkImageUsageFlags swapchainUsage = mSwapchain->usage();

	// Get supported format for colour attachment
	mColourFormat = chooseSupportedFormat(
		{ VK_FORMAT_R8G8B8A8_UNORM },
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	);

	// Get supported format for depth buffer
	mDepthFormat = chooseSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	for (auto& image : mSwapchain->images())
	{
		// IMAGES + RENDERTARGET + RESOURCE REFERENCE
		
		// 0 - swapchain image
		Image swapchainImage(*mDevice,
			image,
			swapchainExtent,
			swapchainFormat,
			swapchainUsage);

		// 1 - colour image
		Image colourImage(*mDevice,
			swapchainExtent,
			mColourFormat,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// 2 - depth image
		Image depthImage(*mDevice,
			swapchainExtent,
			mDepthFormat,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


		std::vector<Image> renderTargetImages;
		renderTargetImages.push_back(std::move(swapchainImage));
		renderTargetImages.push_back(std::move(colourImage));
		renderTargetImages.push_back(std::move(depthImage));

		// Create Render Target + Frame
		std::unique_ptr<RenderTarget> renderTarget = std::make_unique<RenderTarget>(std::move(renderTargetImages));
		mFrames.push_back(std::make_unique<Frame>(*mDevice, std::move(renderTarget), mThreadCount));

		// TODO: make creation of resource references more readable e.g. save image indices in variables
		// Create resource reference for descriptor sets
		auto& imageViews = mFrames.back()->renderTarget().imageViews();
		mAttachmentResources.push_back(std::make_unique<DescriptorResourceReference>());
		mAttachmentResources.back()->bindInputImage(imageViews[1],
			0,
			0);
		mAttachmentResources.back()->bindInputImage(imageViews[2],
			1,
			0);
	}
}

void VulkanRenderer::createRenderPass()
{
	// Array of our subpasses
	std::array<VkSubpassDescription, 2> subpasses{};

	// ATTACHMENTS
	// SUBPASS 1 ATTACHMENTS + REFERENCES (INPUT ATTACHMENTS)

	// Colour Attachment (Input)
	VkAttachmentDescription colourAttachment = {};
	colourAttachment.format = mColourFormat;
	colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colourAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Depth attachment  (Input)
	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = mDepthFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Colour Attachment (Input) Reference
	VkAttachmentReference colourAttachmentReference = {};
	colourAttachmentReference.attachment = 1;				// Numbering based on how attachments to framebuffer were defined	
	colourAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Depth Attachment (Input) Reference
	VkAttachmentReference depthAttachmentReference = {};
	depthAttachmentReference.attachment = 2;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = 1;
	subpasses[0].pColorAttachments = &colourAttachmentReference;
	subpasses[0].pDepthStencilAttachment = &depthAttachmentReference;

	// SUBPASS 2 ATTACHMENTS + REFERENCES

	// Swapchain Colour attachment
	VkAttachmentDescription swapchainColourAttachment = {};
	swapchainColourAttachment.format = mSwapchain->format();					// Format to use for attachment
	swapchainColourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;					// Number of samples to write for multisampling
	swapchainColourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;				// Describes what to do with attachment before rendering
	swapchainColourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;			// Describes what to do with attachment after rendering
	swapchainColourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;	// Describes what to do with stencil before rendering
	swapchainColourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	// Describes what to do with stencil after rendering

	// Framebuffer data will be stored as an image, but images can be given different data layouts
	// to give optimal use for certain operations
	swapchainColourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// Image data layout before render pass starts
	swapchainColourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;		// Image data layout after render pass

	// Attachment reference uses an attachment index that refers to index in the attachment list passed to the renderPassCreateInfo
	VkAttachmentReference swapchainColourAttachmentReference = {};
	swapchainColourAttachmentReference.attachment = 0;
	swapchainColourAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// References to attachments that subpass which will take input from
	std::array<VkAttachmentReference, 2> inputReferences;
	inputReferences[0].attachment = 1;
	inputReferences[0].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	inputReferences[1].attachment = 2;
	inputReferences[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


	// Set up Subpass 2
	subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[1].colorAttachmentCount = 1;
	subpasses[1].pColorAttachments = &swapchainColourAttachmentReference;
	subpasses[1].inputAttachmentCount = static_cast<uint32_t>(inputReferences.size());
	subpasses[1].pInputAttachments = inputReferences.data();


	// SUBPASS DEPENDENCIES

	// Need to determine when layout transitions occur using subpass dependencies
	std::array<VkSubpassDependency, 3> subpassDependencies;

	// Conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// Transition must happen after...
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;						// Subpass index (VK_SUBPASS_EXTERNAL = special calue meaning outside of renderpass
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;		// Pipeline stage
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;				// Stage access mask (memory access)

	// But transition must happen before...
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = 0;

	// Subpass 1 layout (colour/depth) to Subpass 2 layout (shader read)
	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[1].dstSubpass = 1;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	subpassDependencies[1].dependencyFlags = 0;



	// Conversion from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	// Transition must happen after...
	subpassDependencies[2].srcSubpass = 0;						// Subpass index (VK_SUBPASS_EXTERNAL = special calue meaning outside of renderpass
	subpassDependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;		// Pipeline stage
	subpassDependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;				// Stage access mask (memory access)

	// But transition must happen before...
	subpassDependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[2].dependencyFlags = 0;

	std::array<VkAttachmentDescription, 3> renderPassAttachments = { swapchainColourAttachment, colourAttachment, depthAttachment };

	// Create info for renderpass
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(renderPassAttachments.size());
	renderPassCreateInfo.pAttachments = renderPassAttachments.data();
	renderPassCreateInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
	renderPassCreateInfo.pSubpasses = subpasses.data();
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
	renderPassCreateInfo.pDependencies = subpassDependencies.data();

	VkResult result = vkCreateRenderPass(mDevice->logicalDevice(), &renderPassCreateInfo, nullptr, &mRenderPass);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Render Pass !");
	}


}

void VulkanRenderer::createDescriptorSetLayouts()
{


	// Create shader resource objects
	// UNIFORM BUFFER
	ShaderResource vpBuffer(0,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		1,
		VK_SHADER_STAGE_VERTEX_BIT);

	std::vector<ShaderResource> vpResources;

	vpResources.push_back(vpBuffer);

	mUniformSetLayout = std::make_unique<DescriptorSetLayout>(*mDevice, 0, vpResources);


	// TEXTURE SAMPLER
	ShaderResource textureSampler(0,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		1,
		VK_SHADER_STAGE_FRAGMENT_BIT);

	std::vector<ShaderResource> samplerResources;

	samplerResources.push_back(textureSampler);

	mSamplerSetLayout = (std::make_unique<DescriptorSetLayout>(*mDevice, 1, samplerResources));

	// INPUT ATTACHMENTS
	ShaderResource depthAttachment(0,
		VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
		1,
		VK_SHADER_STAGE_FRAGMENT_BIT);

	ShaderResource colourAttachment(1,
		VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
		1,
		VK_SHADER_STAGE_FRAGMENT_BIT);

	std::vector<ShaderResource> attachmentResources;

	attachmentResources.push_back(depthAttachment);
	attachmentResources.push_back(colourAttachment);

	mAttachmentSetLayout = (std::make_unique<DescriptorSetLayout>(*mDevice, 2, attachmentResources));	

}

void VulkanRenderer::createPushConstantRange()
{
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;	// Shader stage push constant will go to
	pushConstantRange.offset = 0;								// Offset into given data to pass to push constant
	pushConstantRange.size = sizeof(glm::mat4);					// Size of data being passed
}

void VulkanRenderer::createGraphicsPipeline()
{
	// Read in SPIR-V code of shaders
	auto vertexShaderCode = readFile("Shaders/vert.spv");
	auto fragmentShaderCode = readFile("Shaders/frag.spv");

	// Create shader modules
	VkShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);
	VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);

	// -- SHADER STAGE CREATION INFORMATION --
	// Vertex Stage creation information
	VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {};
	vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;				// Shader stage name
	vertexShaderCreateInfo.module = vertexShaderModule;						// Shader module to be used by stage
	vertexShaderCreateInfo.pName = "main";									// Entry point into shader	

	// Fragment Stage creation information
	VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo = {};
	fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;				// Shader stage name
	fragmentShaderCreateInfo.module = fragmentShaderModule;						// Shader module to be used by stage
	fragmentShaderCreateInfo.pName = "main";										// Entry point into shader

	// Put shader stage creation info into array
	// Graphics Pipeline creation info requires array of shader stage creates
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderCreateInfo, fragmentShaderCreateInfo };

	// How the data for a single vertex (including info such as position, colour, texture coords, normals, etc) is as a whole
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;								// Can bind multiple streams of data, this defines which one
	bindingDescription.stride = sizeof(Vertex);					// Size of a single vertex object
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;	// How to move between data after each vertex
																// VK_VERTEX_INPUT_RATE_INDEX		: Move on the the next vertex
																// VK_VERTEX_INPUT_RATE_INSTANCE	: Move to a vertex for the next mInstance

	// How the data for an attribute is defined within a vertex
	std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions;

	// Position Attribute
	attributeDescriptions[0].binding = 0;							// Which binding the data is at (Should be the same as above)
	attributeDescriptions[0].location = 0;							// Location in shader where data will be read from
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;	// Format the data will take (also helps define size of data)
	attributeDescriptions[0].offset = offsetof(Vertex, pos);		// Where this attribute is defined in the data for a single vertex

	// Colour attribute
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, col);

	// Texture attribute
	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, tex);

	// -- VERTEX INPUT --
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;					// List of vertex binding descriptions (data spacing/stride info)
	vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();		// List of Vertex Attribute descriptions (data format and where to bind to and from)


	// -- INPUT ASSEMBLY --
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;			// Primitive type to assemble vertices as
	inputAssembly.primitiveRestartEnable = VK_FALSE;						// Allow overriding of "strip" topology to start new primitives


	// -- VIEWPORT & SCISSORS
	// Create a viewport info struct
	VkViewport viewport = {};
	viewport.x = 0.0f;									// x start coordinate
	viewport.y = 0.0f;									// y start coordinate
	viewport.width = (float)mSwapchain->extent().width;		// width of viewport
	viewport.height = (float)mSwapchain->extent().height;	// height of viewport
	viewport.minDepth = 0.0f;							// min framebuffer depth
	viewport.maxDepth = 1.0f;							// max framebuffer depth

	// Create a scissor info struct
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };				// Offset to use region from
	scissor.extent = mSwapchain->extent();		// Extent to describe region to use, starting at offset

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;

	/*
	// -- DYNAMIC STATES --
	// Dynamic states to enable
	std::vector<VkDynamicState> dynamicStateEnables;
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);		// Dynamic Viewport : Can resize in command buffer with vkCmdSetViewport(commandbuffer, 0, 1, &viewport);
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);		// Dynamic Scissor	: Can resize in command buffer with vkCmdSetScissor(commandbuffer, 0, 1, &scissor);

	// Dynamic State creation info
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
	dynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();
	*/

	// -- RASTERISER --
	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
	rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.depthClampEnable = VK_FALSE;			// Change if fragments beyond near/far okanes are clipped (default) or clamped to plane
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;	// Whether to discard data and skip rasterizer. Never creates fragments, only suitable for pipeline without framebuffer
	rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;	// How to handle filling points between vertices ->anything other than fill requires a feature
	rasterizerCreateInfo.lineWidth = 1.0f;						// How thick lines should be when drawn
	rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;		// Which face of a tri to cull
	rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;	// Winding to determine which side is front
	rasterizerCreateInfo.depthBiasEnable = VK_FALSE;			// Whether to add depth bias to fragments (good for stopping "shadow acne" in shadow mapping)


	// -- MULTISAMPLING --
	VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {};
	multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;						// Enable multisample shading or not
	multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;		// Number of samples to use per fragment


	// -- BLENDING --
	// Blending decides how to blend a new colour being written to a fragment, with the old value

	// Blend Attachment State (how blending is handled)
	VkPipelineColorBlendAttachmentState colourState = {};
	colourState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT	// Colours to apply blending too
		| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colourState.blendEnable = VK_TRUE;													// Enable blending

	// Blending uses equation: (srcColorBlendFactor * new colour) colorBlendOp (dstColorBlendFactor * old colour)
	colourState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colourState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colourState.colorBlendOp = VK_BLEND_OP_ADD;

	// Summarised: (VK_BLEND_FACTOR_SRC_ALPHA * new colour) + (VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA * old colour)
	//			   (new colour alpha * new colour) + ((1 - new colour alpha) * old colour)

	colourState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colourState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colourState.alphaBlendOp = VK_BLEND_OP_ADD;
	// Summarised: (1 * new alpha) + (0 * old alpha) = new alpha

	VkPipelineColorBlendStateCreateInfo colourBlendingCreateInfo = {};
	colourBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colourBlendingCreateInfo.logicOpEnable = VK_FALSE;				// Alternative to calculations is to use logical operations
	colourBlendingCreateInfo.attachmentCount = 1;
	colourBlendingCreateInfo.pAttachments = &colourState;

	// -- PIPELINE LAYOUT --
	std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = { mUniformSetLayout->handle(), mSamplerSetLayout->handle() };
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

	//Create Pipeline Layout
	VkResult result = vkCreatePipelineLayout(mDevice->logicalDevice(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Pipeline Layout!");
	}

	// -- DEPTH STENCIL TESTING --
	// TODO: setup depth stencil testing
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.depthTestEnable = VK_TRUE;				// Enable depth checking to determine fragment write
	depthStencilCreateInfo.depthWriteEnable = VK_TRUE;				// Enable writing to depth buffer (to replace old values)
	depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;		// Comparison operation that allows and an overwrite (is in front)
	depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;		// Depth bounds test: Does the depth value exist between two bounds
	depthStencilCreateInfo.stencilTestEnable = VK_FALSE;			// Enable Stencil Test

	// --GRAPHICS PIPELINE CREATION
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = 2;									// Number of shader stages
	pipelineCreateInfo.pStages = shaderStages;							// List of shader stages
	pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;		// All the fixed function pipeline stages
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	pipelineCreateInfo.pDynamicState = nullptr;
	pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
	pipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
	pipelineCreateInfo.pColorBlendState = &colourBlendingCreateInfo;
	pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
	pipelineCreateInfo.layout = pipelineLayout;						// Pipeline layout pipeline should use
	pipelineCreateInfo.renderPass = mRenderPass;						// Render pass description the pipeline is compatible with
	pipelineCreateInfo.subpass = 0;									// Subpass of render pass to use with pipeline

	// Pipeline derivatives : Can create multiple pipeline which derive from one another for optimisation
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;		// Existind pipeline to derive from...
	pipelineCreateInfo.basePipelineIndex = -1;					// or index of pipeline being created to derive from (in case creating multiple at once)

	result = vkCreateGraphicsPipelines(mDevice->logicalDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Graphics Pipeline!");
	}

	// Destroy Shader Modules, no longer needed after Pipeline created
	vkDestroyShaderModule(mDevice->logicalDevice(), vertexShaderModule, nullptr);
	vkDestroyShaderModule(mDevice->logicalDevice(), fragmentShaderModule, nullptr);

	// CREATE SECOND PASS PIPELINE
	// Second pass shaders
	auto secondVertexShaderCode = readFile("Shaders/second_vert.spv");
	auto secondFragmentShaderCode = readFile("Shaders/second_frag.spv");

	// Build shaders
	VkShaderModule secondVertexShaderModule = createShaderModule(secondVertexShaderCode);
	VkShaderModule secondFragmentShaderModule = createShaderModule(secondFragmentShaderCode);

	vertexShaderCreateInfo.module = secondVertexShaderModule;
	fragmentShaderCreateInfo.module = secondFragmentShaderModule;

	VkPipelineShaderStageCreateInfo secondShaderStages[] = { vertexShaderCreateInfo, fragmentShaderCreateInfo };

	// No vertex data for second pass
	vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
	vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
	vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;

	// Don't want to write to depth buffer
	depthStencilCreateInfo.depthWriteEnable = VK_FALSE;


	VkDescriptorSetLayout descriptorSetLayout = mAttachmentSetLayout->handle();
	VkPipelineLayoutCreateInfo secondPipelineLayoutCreateInfo = {};
	secondPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	secondPipelineLayoutCreateInfo.setLayoutCount = 1;
	secondPipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
	secondPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	secondPipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	result = vkCreatePipelineLayout(mDevice->logicalDevice(), &secondPipelineLayoutCreateInfo, nullptr, &secondPipelineLayout);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Pipeline Layout!");
	}

	pipelineCreateInfo.pStages = secondShaderStages;		// Update second shader stage list
	pipelineCreateInfo.layout = secondPipelineLayout;		// Change pipeline layout for input attachment descriptor sets
	pipelineCreateInfo.subpass = 1;							// Use second subpass

	// Create second pipeline
	result = vkCreateGraphicsPipelines(mDevice->logicalDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &secondPipeline);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Graphics Pipeline!");
	}

	// Destroy second shader modules
	vkDestroyShaderModule(mDevice->logicalDevice(), secondFragmentShaderModule, nullptr);
	vkDestroyShaderModule(mDevice->logicalDevice(), secondVertexShaderModule, nullptr);
}


//void VulkanRenderer::createSynchronation()
//{
//	//// Semaphore creation information
//	//VkSemaphoreCreateInfo semaphoreCreateInfo = {};
//	//semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
//
//	//VkResult result = vkCreateSemaphore(mDevice->logicalDevice(), &semaphoreCreateInfo, nullptr, &imageAcquired);
//	//if (result != VK_SUCCESS)
//	//{
//	//	throw std::runtime_error("Failed to create a Semaphore!");
//	//}
//}

void VulkanRenderer::createFramebuffers()
{
	for (auto& frame : mFrames)
	{
		auto& renderTarget = frame->renderTarget();

		mFramebuffers.push_back(std::make_unique<Framebuffer>(*mDevice, mRenderPass, renderTarget));
	}
}

void VulkanRenderer::createTextureSampler()
{
	mTextureSampler = std::make_unique<Sampler>(*mDevice);
}

void VulkanRenderer::createUniformBuffers()
{

	// ViewProjection Buffer size
	VkDeviceSize vpBufferSize = sizeof(UboViewProjection);

	// Create uniform buffers
	for (size_t i = 0; i < mSwapchain->details().imageCount; ++i)
	{
		mUniformBuffers.push_back(std::make_unique<Buffer>(*mDevice,
			vpBufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
	}
	
	// TODO : MAP RESOURCE SET
	// TODO: perform in descriptor set creation?

}

void VulkanRenderer::createDescriptorPools()
{
	// CREATE UNIFORM DESCRIPTOR POOL
	mUniformDescriptorPool = std::make_unique<DescriptorPool>(*mDevice, *mUniformSetLayout, mSwapchain->imageCount());


	// TODO : maxsets here must be changed
	// CREATE SAMPLER DESCRIPTOR POOL
	mSamplerDescriptorPool = std::make_unique<DescriptorPool>(*mDevice, *mSamplerSetLayout, mSwapchain->imageCount());

	// CREATE INPUT ATTACHMENT DESCRIPTOR POOL
	mAttachmentDescriptorPool = std::make_unique<DescriptorPool>(*mDevice, *mAttachmentSetLayout, mSwapchain->imageCount());

}

//void VulkanRenderer::createDescriptorResourceReferences()
//{
//
//
//	// UNIFORM BUFFER
//
//	// SAMPLER
//
//	// ATTACHMENTS
//}


void VulkanRenderer::createUniformDescriptorSets()
{
	// CREATE SETS 
	for (size_t i = 0; i < mSwapchain->details().imageCount; ++i)
	{
		mUniformResources.push_back(std::make_unique<DescriptorResourceReference>());
		mUniformResources.back()->bindBuffer(*mUniformBuffers[i],
			0,
			sizeof(UboViewProjection),
			0,
			0);

		VkDescriptorBufferInfo vpBufferInfo = {};
		mUniformResources[i]->generateDescriptorBufferInfo(vpBufferInfo, 0, 0);
		BindingMap<VkDescriptorBufferInfo> uniformBufferInfos;

		uniformBufferInfos[0][0] = vpBufferInfo;

		mUniformDescriptorSets.push_back(std::make_unique<DescriptorSet>(*mDevice, *mUniformSetLayout, *mUniformDescriptorPool, uniformBufferInfos));

		// Update the descriptor sets with new buffer/binding info
		std::vector<uint32_t> bindingsToUpdate = { 0 };
		mUniformDescriptorSets.back()->update(bindingsToUpdate);
	}
}

void VulkanRenderer::createInputDescriptorSets()
{
	// CREATE SETS
	for (size_t i = 0; i < mSwapchain->imageCount(); ++i)
	{
		BindingMap<VkDescriptorImageInfo> imageInfos{};

		VkDescriptorImageInfo imageInfo = {};
		mAttachmentResources[i]->generateDescriptorImageInfo(imageInfo, 0, 0);
		imageInfos[0][0] = imageInfo;
		mAttachmentResources[i]->generateDescriptorImageInfo(imageInfo, 1, 0);
		imageInfos[1][0] = imageInfo;

		mAttachmentDescriptorSets.push_back(std::make_unique<DescriptorSet>(*mDevice, *mAttachmentSetLayout, *mAttachmentDescriptorPool, imageInfos));

		// Update the descriptor sets with new image/binding info
		std::vector<uint32_t> bindingsToUpdate = { 0, 1 };
		mAttachmentDescriptorSets.back()->update(bindingsToUpdate);
	}
}

void VulkanRenderer::updateUniformBuffers(uint32_t imageIndex)
{
	// Copy VP data
	void* data = mUniformBuffers[imageIndex]->map();
	memcpy(data, &uboViewProjection, sizeof(UboViewProjection));
	mUniformBuffers[imageIndex]->unmap();
}

// TODO: changes threads to split load for number of meshes rather than models
void VulkanRenderer::recordCommands(CommandBuffer& primaryCmdBuffer, uint32_t currentImage) // Current image is swapchain index
{
	auto& frame = *mFrames[currentImage];
	auto& framebuffer = mFramebuffers[currentImage];

	primaryCmdBuffer.beginRecording();


	std::vector<VkClearValue> clearValues;
	clearValues.resize(frame.renderTarget().imageViews().size());
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };				// Clear values for swapchain image (colour)
	clearValues[1].color = { 0.6f, 0.65f, 0.4f, 1.0f };				// Clear values for attachment 1 (colour)
	clearValues[2].depthStencil.depth = 1.0f;						// Clear values for attachment 2 (depth)

	primaryCmdBuffer.beginRenderPass(frame.renderTarget(),
		mRenderPass,
		*framebuffer,
		clearValues,
		VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	// Split objects to draw equally between threads
	uint32_t numModels = modelList.size();

	float avgObjectsPerBuffer = static_cast<float>(numModels) / mThreadCount;

	uint32_t objectsPerBuffer = static_cast<uint32_t> (std::floor(avgObjectsPerBuffer));
	uint32_t remainderObjects = numModels % objectsPerBuffer;
	uint32_t objectStart = 0;

	// Vector for results of tasks pushed to threadpool
	std::vector<std::future<CommandBuffer*>> futureSecondaryCommandBuffers;

	for (uint32_t i = 0; i < mThreadCount; ++i)
	{
		// Get the end index for the last mesh which will be handled by this buffer
		uint32_t objectEnd = std::min(numModels, objectStart + objectsPerBuffer);

		// If there are still remainder draws then add a draw to this command buffer
		// Latter command buffers may contain fewer draws
		if (remainderObjects > 0)
		{
			objectEnd++;
			remainderObjects--;
		}

		// Push lambda function to threadpool for running
		auto futureResult = mThreadPool.push([=, &primaryCmdBuffer](size_t threadIndex) {
			return recordSecondaryCommandBuffers(&primaryCmdBuffer, objectStart, objectEnd, threadIndex);
		});

		futureSecondaryCommandBuffers.push_back(std::move(futureResult));

		objectStart = objectEnd;
	}

	std::vector<CommandBuffer* > secondaryCommandBufferPtrs;

	for (auto& fut : futureSecondaryCommandBuffers)
	{
		secondaryCommandBufferPtrs.push_back(fut.get());
	}

	// Submit the secondary command buffers to the primary command buffer.
	primaryCmdBuffer.executeCommands(secondaryCommandBufferPtrs);
	
	primaryCmdBuffer.nextSubpass(VK_SUBPASS_CONTENTS_INLINE);

	primaryCmdBuffer.bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, secondPipeline);

	std::vector<std::reference_wrapper<const DescriptorSet>> descriptorGroup{ *mAttachmentDescriptorSets[currentImage] };

	primaryCmdBuffer.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, secondPipelineLayout,
		0, descriptorGroup);

	primaryCmdBuffer.draw(3, 1, 0, 0);

	// End Render Pass
	primaryCmdBuffer.endRenderPass();

	// Stop recording to primary command buffers
	primaryCmdBuffer.endRecording();

}


CommandBuffer* VulkanRenderer::recordSecondaryCommandBuffers(CommandBuffer* primaryCommandBuffer, uint32_t objectStart, uint32_t objectEnd, size_t threadIndex)
{
	auto& frame = mFrames[activeFrameIndex];

	auto& queue = mDevice->queue(mGraphicsQueueFamily, 0);

	CommandBuffer& cmdBuffer = frame->requestCommandBuffer(queue, VK_COMMAND_BUFFER_LEVEL_SECONDARY, threadIndex);

	cmdBuffer.beginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT 
		| VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT, primaryCommandBuffer);

	cmdBuffer.bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	for (uint32_t i = objectStart; i < objectEnd; ++i)
	{
		MeshModel* thisModel = &modelList[i];
		int modelDataIndex = thisModel->getMeshDataID();
		MeshModelData* thisData = &modelDataList[modelDataIndex];

		// "Push" constants to given shader stage directly
		cmdBuffer.pushConstant(pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT,
			thisModel->getModel());


		for (size_t j = 0; j < thisData->getMeshCount(); ++j)
		{
			Mesh* thisMesh = thisData->getMesh(j);

			std::vector<std::reference_wrapper<const Buffer>> vertexBuffers{ thisMesh->vertexBuffer() };	// Buffers to bind
			std::vector<VkDeviceSize> offsets{ 0 };															// Offsets into buffers being bound
			cmdBuffer.bindVertexBuffers(0, vertexBuffers, offsets);

			cmdBuffer.bindIndexBuffer(thisMesh->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);

			std::vector<std::reference_wrapper<const DescriptorSet>> descriptorSetGroup{ *mUniformDescriptorSets[activeFrameIndex],
				*mTextureDescriptorSets[thisMesh->texId()] };

			cmdBuffer.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
				0, descriptorSetGroup);

			// Execute pipeline
			cmdBuffer.drawIndexed(thisMesh->indexCount(), 1, 0, 0, 0);
		}
	}

	// Stop recording to primary command buffers
	cmdBuffer.endRecording();

	return &cmdBuffer;

}

void VulkanRenderer::allocateDynamicBufferTransferSpace()
{
	/*
	// Calculate alignment of model data
	modelUniformAlignment = (sizeof(UboModel) + minUniformBufferOffset - 1)
							& ~(minUniformBufferOffset - 1);

	// Create space in memory to hold dynamic buffer that is aligned to our required alignment and holds MAX_OBJECTS
	modelTransferSpace = (UboModel*)_aligned_malloc(modelUniformAlignment * MAX_OBJECTS, modelUniformAlignment);
	*/
}

VkFormat VulkanRenderer::chooseSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags)
{
	// Loop through the options and find a compatible one
	for (VkFormat format : formats)
	{
		// Get properties for given format on this device
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(mDevice->physicalDevice(), format, &properties);

		// Depending on tiling choice, need to check for different bit flag
		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & featureFlags) == featureFlags)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & featureFlags) == featureFlags)
		{
			return format;
		}
	}

	throw std::runtime_error("Failed to find a matching format!");
}

VkShaderModule VulkanRenderer::createShaderModule(const std::vector<char>& code)
{
	// Shader module creation info
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = code.size();										// Size of code
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());		// Pointer to code (of uint32_t pointer type)

	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(mDevice->logicalDevice(), &shaderModuleCreateInfo, nullptr, &shaderModule);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a shader module!");
	}

	return shaderModule;
}

int VulkanRenderer::createTexture(std::string fileName)
{
	// Load in the image file
	int width, height;
	VkDeviceSize imageSize;
	stbi_uc* textureData = loadTextureFile(fileName, width, height, imageSize);

	std::unique_ptr<Texture>  texture = std::make_unique<Texture>(*mDevice, textureData, width, height, imageSize);

	// Add texture to map of textures
	int textureID = texture->textureID();;

	
	// Create Texture Descriptor
	int descriptorLoc = createTextureDescriptor(*texture);

	mTextures[textureID] = std::move(texture);
	assert(!texture); // just checking ownership of the texture ptr has moved to the map

	// Return location of set with texture
	return descriptorLoc;
}

int VulkanRenderer::createTextureDescriptor(const Texture& texture)
{
	// Create descriptor resource reference
	// TODO : make this a map?
	mSamplerResources.push_back(std::make_unique<DescriptorResourceReference>());
	mSamplerResources.back()->bindImage(texture.imageView(), *mTextureSampler, 0, 0);
	
	VkDescriptorImageInfo imageInfo = {};
	mSamplerResources.back()->generateDescriptorImageInfo(imageInfo, 0, 0);

	BindingMap<VkDescriptorImageInfo> imageInfos;
	imageInfos[0][0] = imageInfo;

	mTextureDescriptorSets.push_back(std::make_unique<DescriptorSet>(*mDevice, *mSamplerSetLayout, *mSamplerDescriptorPool, imageInfos));

	// Update the descriptor sets with new buffer/binding info
	std::vector<uint32_t> bindingsToUpdate = { 0 };
	mTextureDescriptorSets.back()->update(bindingsToUpdate);

	// Return descriptor set location
	return mTextureDescriptorSets.size() - 1;


}

stbi_uc* VulkanRenderer::loadTextureFile(std::string fileName, int& width, int& height, VkDeviceSize& imageSize)
{
	// Number of channels image uses
	int channels;

	// Load pixel data for image
	std::string fileLoc = "Textures/" + fileName;
	stbi_uc* image = stbi_load(fileLoc.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	if (!image)
	{
		throw std::runtime_error("Failed to load a Texture File! (" + fileName + ")");
	}

	// calculate image size using given and known data (note 4 is for RGB and A channels)
	imageSize = width * height * 4;

	return image;
}

int VulkanRenderer::loadMeshModelData(std::string modelFile)
{
	// Import model "scene"
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(modelFile, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

	if (!scene)
	{
		throw std::runtime_error("Failed to load model! (" + modelFile + ")");
	}

	// Get vector of all materials with 1:1 ID placement
	std::vector<std::string> textureNames = MeshModelData::LoadMaterials(scene);

	// Conversion from the materials list IDs to our descriptor Array IDs
	std::vector<int> matToTex(textureNames.size());

	// Loop over textureNames and create textures for them
	for (size_t i = 0; i < textureNames.size(); ++i)
	{
		// If material has no texture, set '0' to indicate no texture, texture 0 will be reserved for a default texture
		if (textureNames[i].empty())
		{
			matToTex[i] = 0;
		}
		else
		{
			// Otherwise, create texture and set value to index of new texture
			matToTex[i] = createTexture(textureNames[i]);
		}
	}

	// Load in all our meshes
	std::vector<Mesh*> modelMeshes = MeshModelData::LoadNode(*mDevice, scene->mRootNode, scene, matToTex);

	// Create mesh model and add to list
	MeshModelData meshModelData = MeshModelData(modelMeshes);
	modelDataList.push_back(meshModelData);

	return modelDataList.size() - 1;
}

int VulkanRenderer::createModel(int modelDataIndex)
{
	MeshModel newModel = MeshModel(modelDataIndex);

	modelList.push_back(newModel);

	return modelList.size() - 1;
}

void VulkanRenderer::getWindowExtent(VkExtent2D& windowExtent)
{
	// Get window size
	int width, height;
	glfwGetFramebufferSize(mWindow, &width, &height);

	// Create new extent using window size
	windowExtent.width = static_cast<uint32_t>(width);
	windowExtent.height = static_cast<uint32_t>(height);
}



