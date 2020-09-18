#include "VulkanRenderer.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

VulkanRenderer::VulkanRenderer()
{
}

int VulkanRenderer::init(GLFWwindow* newWindow)
{
	window = newWindow;

	try {
		mThreadCount = std::thread::hardware_concurrency();
		/*createThreadData();*/
		createInstance();				// LEAVE
		setupDebugMessenger();			// LEAVE
		createSurface();				// LEAVE
		createDevice();					// COMPLETE
		createSwapChain();				// COMPLETE
		createPerFrameObjects();		// TODO : still need to create frame with descriptor sets, command buffers etc
		//createColourBufferImage();	// Abstract to create renderpass images
		//createDepthBufferImage();		// abstract to create renderpass images
		createRenderPass();				// Change to use rendertarget and subpass objects
		createDescriptorSetLayouts();
		createPushConstantRange();
		createGraphicsPipeline();
		//createFrameBuffers();
		createFramebuffers();
		/*createCommandPools();
		createCommandBuffers();*/
		createTextureSampler();
		//allocateDynamicBufferTransferSpace();
		createUniformBuffers();
		createDescriptorPools();
		//createDescriptorResourceReferences();
		createUniformDescriptorSets();
		createInputDescriptorSets();
		createSynchronation();			// TODO : abstract to a pool
		createCamera(90.0f);

		//createTexture("default_checker.png");
		createTexture("default_black.png");

	}
	catch (const std::runtime_error& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}


	return 0;
}

void VulkanRenderer::updateModel(int modelId, glm::mat4 newModel)
{
	if (modelId >= modelList.size()) return;

	modelList[modelId].setModel(newModel);
}

void VulkanRenderer::createCamera(float FoVinDegrees)
{
	uboViewProjection.projection = glm::perspective(glm::radians(FoVinDegrees), (float)swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 100.0f);
	uboViewProjection.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 20.0f), glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	uboViewProjection.projection[1][1] *= -1;
}

void VulkanRenderer::updateCameraView(mat4 newView)
{
	uboViewProjection.view = newView;
}

void VulkanRenderer::draw()
{
	/* BEGIN ABSTRACT TO SWAPCHAIN ACQUIRE NEXT IMAGE*/
	// -- GET NEXT IMAGE -- **DO NOT REARRANGE ORDER - FENCE CHANGES MUST BE HANDLED FIRST**
	// Wait for given fence to signal (open) from last draw before continuing
	vkWaitForFences(mDevice->logicalDevice(), 1, &drawFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());		// ANALOGY : Wait until this fence is open (freezes the code)
	// Manually reset (close) fences
	vkResetFences(mDevice->logicalDevice(), 1, &drawFences[currentFrame]);														// Close fence behind you

	// Get index of next image to be drawn to and signal semapphore when ready to be drawn to
	uint32_t imageIndex;
	vkAcquireNextImageKHR(mDevice->logicalDevice(), mSwapchain->handle(), std::numeric_limits<uint64_t>::max(), imageAvailable[currentFrame], VK_NULL_HANDLE, &imageIndex);
	/* END ABSTRACT */

	recordCommands(imageIndex);		// Only record commands once the image at imageIndex is available (not being used by the queue)

	updateUniformBuffers(imageIndex);

	// -- SUBMIT COMMAND BUFFER TO RENDER -- 
	// Queue submission information
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;					// Number of semaphores to wait on
	submitInfo.pWaitSemaphores = &imageAvailable[currentFrame];		// Stages to check semaphores at
	VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;					// Number of command buffers to submit
	submitInfo.pCommandBuffers = &primaryCommandBuffers[imageIndex];	// Command buffer to submit
	submitInfo.signalSemaphoreCount = 1;						// Number of semaphores to signal
	submitInfo.pSignalSemaphores = &renderFinished[currentFrame];				// Semaphore to signal when the command buffer finishes

	// submit command buffer to queue
	VkResult result = vkQueueSubmit(mDevice->graphicsQueue(), 1, &submitInfo, drawFences[currentFrame]);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit Command Buffer to Queue!");
	}

	// -- PRESENT RENDERED IMAGE TO SCREEN --
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;						// Number of semaphores to wait on
	presentInfo.pWaitSemaphores = &renderFinished[currentFrame];			// Semaphroes to wait on
	presentInfo.swapchainCount = 1;							// Number of swapchains to present to
	presentInfo.pSwapchains = mSwapchain->handle();					// Swapchains to present images to
	presentInfo.pImageIndices = &imageIndex;				// Index of images in swapchains to present

	// Present image
	result = vkQueuePresentKHR(mDevice->presentationQueue(), &presentInfo);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present Image!");
	}

	// Get next frame (use % MAX_FRAME_DRAWS)
	currentFrame = (currentFrame + 1) % MAX_FRAME_DRAWS;
}

void VulkanRenderer::cleanup()
{
	// Wait until no actions being run on device before destroying
	vkDeviceWaitIdle(mDevice->logicalDevice());

	//_aligned_free(modelTransferSpace);

	for (size_t i = 0; i < modelDataList.size(); ++i)
	{
		modelDataList[i].destroyMeshModel();
	}

	//vkDestroyDescriptorPool(mDevice->logicalDevice(), inputDescriptorPool, nullptr);
	////vkDestroyDescriptorSetLayout(mDevice->logicalDevice(), inputSetLayout, nullptr);

	//vkDestroyDescriptorPool(mDevice->logicalDevice(), samplerDescriptorPool, nullptr);
	//vkDestroyDescriptorSetLayout(mDevice->logicalDevice(), samplerSetLayout, nullptr);

	/*vkDestroySampler(mDevice->logicalDevice(), textureSampler, nullptr);*/

	//for (size_t i = 0; i < textureImages.size(); ++i)
	//{
	//	vkDestroyImageView(mDevice->logicalDevice(), textureImageViews[i], nullptr);
	//	vkDestroyImage(mDevice->logicalDevice(), textureImages[i], nullptr);
	//	vkFreeMemory(mDevice->logicalDevice(), textureImageMemory[i], nullptr);
	//}

	/*for (size_t i = 0; i < depthBufferImage.size(); ++i)
	{
		vkDestroyImageView(mDevice->logicalDevice(), depthBufferImageView[i], nullptr);
		vkDestroyImage(mDevice->logicalDevice(), depthBufferImage[i], nullptr);
		vkFreeMemory(mDevice->logicalDevice(), depthBufferImageMemory[i], nullptr);
	}

	for (size_t i = 0; i < colourBufferImage.size(); ++i)
	{
		vkDestroyImageView(mDevice->logicalDevice(), colourBufferImageView[i], nullptr);
		vkDestroyImage(mDevice->logicalDevice(), colourBufferImage[i], nullptr);
		vkFreeMemory(mDevice->logicalDevice(), colourBufferImageMemory[i], nullptr);
	}*/

	/*vkDestroyDescriptorPool(mDevice->logicalDevice(), descriptorPool, nullptr);*/
	//vkDestroyDescriptorSetLayout(mDevice->logicalDevice(), descriptorSetLayout, nullptr);
	//for (size_t i = 0; i < mSwapchain->details().imageCount; ++i)
	//{
	//	vkDestroyBuffer(mDevice->logicalDevice(), vpUniformBuffer[i], nullptr);
	//	vkFreeMemory(mDevice->logicalDevice(), vpUniformBufferMemory[i], nullptr);
	//	
	//	//vkDestroyBuffer(mDevice->logicalDevice(), modelDUniformBuffer[i], nullptr);
	//	//vkFreeMemory(mDevice->logicalDevice(), modelDUniformBufferMemory[i], nullptr);
	//}

	for (size_t i = 0; i < MAX_FRAME_DRAWS; ++i)
	{
		vkDestroySemaphore(mDevice->logicalDevice(), renderFinished[i], nullptr);
		vkDestroySemaphore(mDevice->logicalDevice(), imageAvailable[i], nullptr);
		vkDestroyFence(mDevice->logicalDevice(), drawFences[i], nullptr);
	}

	//for (auto& frame : frameData)
	//{

	//	for (size_t i = 0; i < threadCount; ++i)
	//	{
	//		vkDestroyCommandPool(mDevice->logicalDevice(), frame.threadData[i].commandPool, nullptr);
	//	}
	//}
	/*vkDestroyCommandPool(mDevice->logicalDevice(), graphicsCommandPool, nullptr);*/

	/*for (auto framebuffer : swapChainFramebuffers)
	{
		vkDestroyFramebuffer(mDevice->logicalDevice(), framebuffer, nullptr);
	}*/
	vkDestroyPipeline(mDevice->logicalDevice(), secondPipeline, nullptr);
	vkDestroyPipelineLayout(mDevice->logicalDevice(), secondPipelineLayout, nullptr);
	vkDestroyPipeline(mDevice->logicalDevice(), graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(mDevice->logicalDevice(), pipelineLayout, nullptr);
	vkDestroyRenderPass(mDevice->logicalDevice(), mRenderPass, nullptr);
	//for (auto image : swapChainImages)
	//{
	//	vkDestroyImageView(mDevice->logicalDevice(), image.imageView, nullptr);
	//}

	//vkDestroySwapchainKHR(mDevice->logicalDevice(), swapchain, nullptr);	// swapchain
	//vkDestroyDevice(mDevice->logicalDevice(), nullptr);  // Abstract to device
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(mInstance, debugMessenger, nullptr);
	}
	//* destructor */
	//vkDestroySurfaceKHR(mInstance, surface, nullptr);
	//vkDestroyInstance(mInstance, nullptr);
}

VulkanRenderer::~VulkanRenderer()
{
	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	vkDestroyInstance(mInstance, nullptr);
}

void VulkanRenderer::createInstance()
{
	// 
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	// Information about the application itself
	// Most data here does not affect the program and is for developer convenience
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan App";				// Custom name of the application
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);	// Custom version of the application
	appInfo.pEngineName = "No Engine";						// Custom engine name
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);		// Custom engine version
	appInfo.apiVersion = VK_API_VERSION_1_2;				// The Vulkan Version

	// Creation information for a VkInstance (Vulkan Instance)
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;



	// Create list to hold mInstance extensions - we must query vulkan for these values
	std::vector<const char*> instanceExtensions = std::vector<const char*>();

	uint32_t glfwExtensionCount = 0;						// GLFW may require multiple extensions
	const char** glfwExtensions;							// Extensions passed as array of cstrings, so need pointer (the array) to pinter (the cstring)

	// Get GLFW extensions
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// Add GLFW extensions to list of extensions
	for (size_t i = 0; i < glfwExtensionCount; ++i)
	{
		instanceExtensions.push_back(glfwExtensions[i]);
	}

	if (enableValidationLayers) {
		instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	// Check mInstance extensions are supported
	if (!checkInstanceExtensionSupport(&instanceExtensions))
	{
		throw std::runtime_error("VkInstance does not support required extrensions!");
	}

	createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());			// Cast to uint32_t as size returns a type t
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();

	// Setup validation layers that the mInstance will use
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
	}

	// Create mInstance - must remember to destroy this object in memory later
	VkResult result = vkCreateInstance(&createInfo, nullptr, &mInstance);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Vulkan instance");
	}

}

// Create interface to the window
void VulkanRenderer::createSurface()
{
	// Create Surface (creates a surface create info struct, runs the create surface function, returns result)
	VkResult result = glfwCreateWindowSurface(mInstance, window, nullptr, &mSurface);


	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a surface!");
	}
}

void VulkanRenderer::createDevice()
{
	mDevice = std::make_unique<Device>(mInstance, mSurface, deviceExtensions);
}

void VulkanRenderer::createSwapChain()
{
	VkExtent2D windowExtent;
	getWindowExtent(windowExtent, window);

	mSwapchain = std::make_unique<Swapchain>(*mDevice, windowExtent, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
}

// Prepare rendertargets and frames
void VulkanRenderer::createPerFrameObjects()
{
	auto& device = mSwapchain->device();
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
		std::vector<Image> renderTargetImages;
		mAttachmentResources.push_back(std::make_unique<DescriptorResourceReference>());

		// 0 - swapchain image
		Image swapchainImage(device,
			image,
			swapchainExtent,
			swapchainFormat,
			swapchainUsage);

		// 1 - colour image
		Image colourImage(device,
			swapchainExtent,
			mColourFormat,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT);

		mAttachmentResources.back()->bindInputImage(colourImage,
			0,
			0);

		// 2 - depth image
		Image depthImage(device,
			swapchainExtent,
			mDepthFormat,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT);

		mAttachmentResources.back()->bindInputImage(depthImage,
			0,
			0);


		renderTargetImages.push_back(swapchainImage);
		renderTargetImages.push_back(colourImage);
		renderTargetImages.push_back(depthImage);


		std::unique_ptr<RenderTarget> renderTarget = std::make_unique<RenderTarget>(renderTargetImages);
		mFrames.push_back(std::make_unique<Frame>(mDevice, renderTarget, mThreadCount));
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
	swapchainColourAttachment.format = swapChainImageFormat;						// Format to use for attachment
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

	mUniformSetLayout = std::make_unique<DescriptorSetLayout>(mDevice, 0, vpResources);


	// TEXTURE SAMPLER
	ShaderResource textureSampler(0,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		1,
		VK_SHADER_STAGE_FRAGMENT_BIT);

	std::vector<ShaderResource> samplerResources;

	samplerResources.push_back(textureSampler);

	mSamplerSetLayout = (std::make_unique<DescriptorSetLayout>(mDevice, 1, samplerResources));

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

	mAttachmentSetLayout = (std::make_unique<DescriptorSetLayout>(mDevice, 2, attachmentResources));	

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
	viewport.width = (float)swapChainExtent.width;		// width of viewport
	viewport.height = (float)swapChainExtent.height;	// height of viewport
	viewport.minDepth = 0.0f;							// min framebuffer depth
	viewport.maxDepth = 1.0f;							// max framebuffer depth

	// Create a scissor info struct
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };				// Offset to use region from
	scissor.extent = swapChainExtent;		// Extent to describe region to use, starting at offset

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

/* abstracted to frames*/
//void VulkanRenderer::createCommandPools()
//{
//	// PRIMARY POOL
//	// Get indices of queue families for device
//	QueueFamilyIndices queueFamilyIndices = mDevice->queueFamilyIndices();
//
//	VkCommandPoolCreateInfo poolInfo = {};
//	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
//	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
//	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;		// Queue family type that buffers from this command pool will use
//
//	// Create a graphics queue family command pool
//	VkResult result = vkCreateCommandPool(mDevice->logicalDevice(), &poolInfo, nullptr, &graphicsCommandPool);
//	if (result != VK_SUCCESS)
//	{
//		throw std::runtime_error("Failed to create a Command Pool!");
//	}
//
//	// COMMAND POOL FOR EACH SECONDARY BUFFER 
//	// (One command pool cannot be accessed by 2 threads simultaneously)
//
//	//secondaryCommandPools.resize(threadCount);
//
//	for (auto& frame : frameData)
//	{
//		for (size_t i = 0; i < threadCount; ++i)
//		{
//			result = vkCreateCommandPool(mDevice->logicalDevice(), &poolInfo, nullptr, &frame.threadData[i].commandPool);
//			if (result != VK_SUCCESS)
//			{
//				throw std::runtime_error("Failed to create a Command Pool!");
//			}
//		}
//	}
//
//
//
//}

//void VulkanRenderer::createCommandBuffers()
//{
//
//	size_t numFrames = swapChainFramebuffers.size();
//
//	// PRIMARY BUFFERS
//	// Resize command buffer count to have one for each framebuffer
//	primaryCommandBuffers.resize(numFrames);
//
//	VkCommandBufferAllocateInfo cbAllocInfo = {};
//	cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//	cbAllocInfo.commandPool = graphicsCommandPool;
//	cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;				// VK_COMMAND_BUFFER_LEVEL_PRIMARY   : buffer you submit directly to the queue. Cannot be called by other buffers.
//																		// VK_COMMAND_BUFFER_LEVEL_SECONDARY : buffer cannot be called directly. Can be called by other buffers via "vkCmdExecuteCommands" when recording commands in primary buffer
//	cbAllocInfo.commandBufferCount = static_cast<uint32_t>(primaryCommandBuffers.size());
//
//	// Allocate command buffers and place handles in array of buffers
//	VkResult result = vkAllocateCommandBuffers(mDevice->logicalDevice(), &cbAllocInfo, primaryCommandBuffers.data());
//	if (result != VK_SUCCESS)
//	{
//		throw std::runtime_error("Failed to allocate Command Buffers!");
//	}
//
//	// SECONDARY BUFFERS
//	numSecondaryBuffers = threadCount;
//	cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
//	cbAllocInfo.commandBufferCount = static_cast<uint32_t>(numSecondaryBuffers);
//
//	for (auto& frame : frameData)
//	{
//		for (size_t i = 0; i < threadCount; ++i)
//		{
//			frame.threadData[i].handle.resize(numSecondaryBuffers);
//
//			cbAllocInfo.commandPool = frame.threadData[i].commandPool;
//
//			// Allocate command buffers and place handles in array of buffers
//			result = vkAllocateCommandBuffers(mDevice->logicalDevice(), &cbAllocInfo, frame.threadData[i].handle.data());
//			if (result != VK_SUCCESS)
//			{
//				throw std::runtime_error("Failed to allocate Command Buffers!");
//			}
//		}
//	}
//
//	
//}

void VulkanRenderer::createSynchronation()
{
	imageAvailable.resize(MAX_FRAME_DRAWS);
	renderFinished.resize(MAX_FRAME_DRAWS);
	drawFences.resize(MAX_FRAME_DRAWS);

	// Fence creation information
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;			// Create fence as signaled (open)

	// Semaphore creation information
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for (size_t i = 0; i < MAX_FRAME_DRAWS; ++i)
	{
		if (vkCreateSemaphore(mDevice->logicalDevice(), &semaphoreCreateInfo, nullptr, &imageAvailable[i]) != VK_SUCCESS ||
			vkCreateSemaphore(mDevice->logicalDevice(), &semaphoreCreateInfo, nullptr, &renderFinished[i]) != VK_SUCCESS ||
			vkCreateFence(mDevice->logicalDevice(), &fenceCreateInfo, nullptr, &drawFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create a Semaphore and/or Fence!");
		}
	}

}

void VulkanRenderer::createFramebuffers()
{
	const VkExtent2D& extent = { mRenderTargets.back()->extent().width, mRenderTargets.back()->extent().height };

	for (auto& renderTarget : mRenderTargets)
	{
		mFramebuffers.push_back(std::make_unique<Framebuffer>(mDevice, extent, renderTarget->imageViews(), mRenderPass));
	}
}

//void VulkanRenderer::createThreadData()
//{
//	threadCount = std::thread::hardware_concurrency();
//	threadPool.resize(threadCount);
//	frameData.resize(mSwapchain->details().imageCount);
//
//	for (auto& frame : frameData)
//	{
//		frame.threadData.resize(threadCount);
//	}
//	
//}

void VulkanRenderer::createTextureSampler()
{
	mTextureSampler = std::make_unique<Sampler>(mDevice);
}

void VulkanRenderer::createUniformBuffers()
{

	// ViewProjection Buffer size
	VkDeviceSize vpBufferSize = sizeof(UboViewProjection);

	// Create uniform buffers
	for (size_t i = 0; i < mSwapchain->details().imageCount; ++i)
	{
		mUniformBuffers.push_back(std::make_unique<Buffer>(mDevice,
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
	mUniformDescriptorPool = std::make_unique<DescriptorPool>(mDevice, mUniformSetLayout, mSwapchain->details().imageCount);

	// CREATE SAMPLER DESCRIPTOR POOL
	mSamplerDescriptorPool = std::make_unique<DescriptorPool>(mDevice, mSamplerSetLayout, mSwapchain->details().imageCount);

	// CREATE INPUT ATTACHMENT DESCRIPTOR POOL
	mAttachmentDescriptorPool = std::make_unique<DescriptorPool>(mDevice, mAttachmentSetLayout, mSwapchain->details().imageCount);

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
		std::vector<VkDescriptorBufferInfo> uniformBufferInfos = { vpBufferInfo };

		mUniformDescriptorSets.push_back(std::make_unique<DescriptorSet>(mDevice, mUniformDescriptorPool, uniformBufferInfos));

		// Update the descriptor sets with new buffer/binding info
		std::vector<uint32_t> bindingsToUpdate = { 0 };
		mUniformDescriptorSets.back()->update(bindingsToUpdate);
	}
}

void VulkanRenderer::createInputDescriptorSets()
{
	// CREATE SETS
	for (size_t i = 0; i < mSwapchain->details().imageCount; ++i)
	{
		std::vector<VkDescriptorImageInfo> attachmentImageInfos;

		VkDescriptorImageInfo imageInfo = {};
		mAttachmentResources[i]->generateDescriptorImageInfo(imageInfo, 0, 0);
		attachmentImageInfos.push_back(imageInfo);
		mAttachmentResources[i]->generateDescriptorImageInfo(imageInfo, 1, 0);
		attachmentImageInfos.push_back(imageInfo);

		mAttachmentDescriptorSets.push_back(std::make_unique<DescriptorSet>(mDevice, mAttachmentDescriptorPool, attachmentImageInfos));

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
void VulkanRenderer::recordCommands(uint32_t currentImage) // Current image is swapchain index
{
	// Information about how to begin each command buffer
	VkCommandBufferBeginInfo bufferBeginInfo = {};
	bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	// Information about how to begin a render pass (only needed for graphical applications)
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = mRenderPass;							// Render pass to begin
	renderPassBeginInfo.renderArea.offset = { 0, 0 };						// Start point of render pass in pixels
	renderPassBeginInfo.renderArea.extent = swapChainExtent;				// Size of region to run render pass on (starting at offset)

	std::array<VkClearValue, 3> clearValues = {};
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };				// Clear values for attachment 1 (colour)
	clearValues[1].color = { 0.6f, 0.65f, 0.4f, 1.0f };				// Clear values for attachment 1 (colour)
	clearValues[2].depthStencil.depth = 1.0f;						// Clear values for attachment 2 (depth)


	renderPassBeginInfo.pClearValues = clearValues.data();							// List of clear values 
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());

	renderPassBeginInfo.framebuffer = mFramebuffers[currentImage]->handle();

	// Start recording commands to command buffer
	VkResult result = vkBeginCommandBuffer(primaryCommandBuffers[currentImage], &bufferBeginInfo);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to start recording a Primary Command Buffer!");
	}

	// Inheritance create info allows secondary buffers to inherit render pass state
	VkCommandBufferInheritanceInfo inheritanceInfo = {};
	inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritanceInfo.renderPass = mRenderPass;
	inheritanceInfo.framebuffer = mFramebuffers[currentImage]->handle();
	inheritanceInfo.subpass = 0;

	VkCommandBufferBeginInfo secondaryBeginInfo = {};
	secondaryBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	secondaryBeginInfo.flags =
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	secondaryBeginInfo.pInheritanceInfo = &inheritanceInfo;

	// Below is indented to indicate that the commands are being recorded in the command buffer

		// Begin Render Pass (Use secondary command buffers to allow for multithreading)
	vkCmdBeginRenderPass(primaryCommandBuffers[currentImage], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
	//vkCmdBeginRenderPass(primaryCommandBuffers[currentImage], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);



		// Split objects to draw equally between threads
	uint32_t numModels = modelList.size();

	float avgObjectsPerBuffer = static_cast<float>(numModels) / threadCount;

	uint32_t objectsPerBuffer = static_cast<uint32_t> (std::floor(avgObjectsPerBuffer));
	uint32_t remainderObjects = numModels % objectsPerBuffer;
	uint32_t objectStart = 0;

	// Vector for results of tasks pushed to threadpool
	std::vector<std::future<VkCommandBuffer*>> futureSecondaryCommandBuffers;

	for (uint32_t i = 0; i < numSecondaryBuffers; ++i)
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
		auto futureResult = threadPool.push([=](size_t threadID) {
			return recordSecondaryCommandBuffers(secondaryBeginInfo, objectStart, objectEnd, currentImage, i, threadID);
			});

		futureSecondaryCommandBuffers.push_back(std::move(futureResult));

		objectStart = objectEnd;
	}

	std::vector<VkCommandBuffer* > secondaryCommandBufferPtrs;

	for (auto& fut : futureSecondaryCommandBuffers)
	{
		secondaryCommandBufferPtrs.push_back(fut.get());
	}


	std::vector<VkCommandBuffer> secondaryCommandBuffers(secondaryCommandBufferPtrs.size(), VK_NULL_HANDLE);
	// transform to a vector of commandbuffers
	std::transform(secondaryCommandBufferPtrs.begin(), secondaryCommandBufferPtrs.end(),
		secondaryCommandBuffers.begin(), [](VkCommandBuffer* item) {return *item; });

	// Submit the secondary command buffers to the primary command buffer.
	vkCmdExecuteCommands(primaryCommandBuffers[currentImage], secondaryCommandBuffers.size(), secondaryCommandBuffers.data());

	// Start second subpass (INLINE here as no multithreading is used and only primary buffers are submitted)
	vkCmdNextSubpass(primaryCommandBuffers[currentImage], VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(primaryCommandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, secondPipeline);
	vkCmdBindDescriptorSets(primaryCommandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, secondPipelineLayout,
		0, 1, &inputDescriptorSets[currentImage], 0, nullptr);
	vkCmdDraw(primaryCommandBuffers[currentImage], 3, 1, 0, 0);

	// End Render Pass
	vkCmdEndRenderPass(primaryCommandBuffers[currentImage]);

	// Stop recording to primary command buffers
	result = vkEndCommandBuffer(primaryCommandBuffers[currentImage]);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to stop recording Command Buffer!");
	}
}


VkCommandBuffer* VulkanRenderer::recordSecondaryCommandBuffers(VkCommandBufferBeginInfo beginInfo, uint32_t objectStart, uint32_t objectEnd, uint32_t currentImage, uint32_t taskIndex, size_t threadID)
{
	ThreadData* thread = &frameData[currentImage].threadData[threadID];

	VkCommandBuffer& cmdBuffer = thread->commandBuffer[taskIndex];

	// Begin recording for each secondary command buffer
	VkResult result = vkBeginCommandBuffer(cmdBuffer, &beginInfo);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to start recording a Secondary Command Buffer!");
	}

	// Bind graphics pipeline to command buffer
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	for (uint32_t i = objectStart; i < objectEnd; ++i)
	{
		MeshModel* thisModel = &modelList[i];
		int modelDataIndex = thisModel->getMeshDataID();
		MeshModelData* thisData = &modelDataList[modelDataIndex];

		// "Push" constants to given shader stage directly
		vkCmdPushConstants(
			cmdBuffer,
			pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT,		// Stage to push constants to
			0,								// Offset of push constants to update
			sizeof(glm::mat4),					// Size of data being pushed
			&thisModel->getModel());		// Actual data being pushed (can be array)

		for (size_t j = 0; j < thisData->getMeshCount(); ++j)
		{
			Mesh* thisMesh = thisData->getMesh(j);

			VkBuffer vertexBuffers[] = { thisMesh->vertexBuffer().handle() };				// Buffers to bind
			VkDeviceSize offsets[] = { 0 };											// Offsets into buffers being bound
			vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);	// Command to bind vertex buffer before drawing with them

			vkCmdBindIndexBuffer(cmdBuffer, thisMesh->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);

			std::array<VkDescriptorSet, 2> descriptorSetGroup = { descriptorSets[currentImage],
				samplerDescriptorSets[thisMesh->texId()] };

			// Bind descriptor sets
			vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
				0, static_cast<uint32_t>(descriptorSetGroup.size()), descriptorSetGroup.data(), 0, nullptr);

			// Execute pipeline
			vkCmdDrawIndexed(cmdBuffer, thisMesh->indexCount(), 1, 0, 0, 0);

		}
	}

	// Stop recording to primary command buffers
	result = vkEndCommandBuffer(cmdBuffer);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to stop recording Command Buffer!");
	}

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

bool VulkanRenderer::checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions)
{
	// Need to get number of extensions to create array of correct size to hold extensions
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);		// Note this is enumerationg over INSTANCE exceptions

	// Create a list of VkExtensionProperties using count
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	// Check if given extensions are in list of available extensions
	for (const auto& checkExtension : *checkExtensions)
	{
		bool hasExtension = false;
		for (const auto& extension : extensions)
		{
			if (strcmp(checkExtension, extension.extensionName))
			{
				hasExtension = true;
				break;
			}
		}

		// Extension not available therefore application will not run
		if (!hasExtension)
		{
			return false;
		}

		// All extensions exist
		return true;
	}


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
	stbi_uc* textureData = Texture::loadTextureFile(fileName, &width, &height, &imageSize);
	// TODO : maybe it is cleaner to just create the texture object by passing it the file name
	// TODO : but I don't want the texture to have to know the file name and I want to leave the opportunity in future to load image data using different libraries

	std::unique_ptr<Texture>  texture = std::make_unique<Texture>(mDevice, textureData, width, height, imageSize);

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
	mSamplerResources.back()->bindImage(texture.image(), *mTextureSampler, 0, 0);
	
	VkDescriptorImageInfo imageInfo = {};
	mSamplerResources.back()->generateDescriptorImageInfo(imageInfo, 0, 0);
	std::vector<VkDescriptorImageInfo> imageInfos = { imageInfo };

	mTextureDescriptorSets.push_back(std::make_unique<DescriptorSet>(mDevice, mSamplerDescriptorPool, imageInfos));

	// Update the descriptor sets with new buffer/binding info
	std::vector<uint32_t> bindingsToUpdate = { 0 };
	mTextureDescriptorSets.back()->update(bindingsToUpdate);

	// Return descriptor set location
	return mTextureDescriptorSets.size() - 1;


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

// Checks that all of the requested layers are available
bool VulkanRenderer::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

void VulkanRenderer::setupDebugMessenger()
{
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);


	if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

void VulkanRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}



