#define STB_IMAGE_IMPLEMENTATION
#include "VulkanRenderer.h"

#include "Mesh.h"
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
#include "Pipeline.h"
#include "PipelineLayout.h"
#include "RenderPass.h"
#include "RenderTarget.h"
#include "ShaderModule.h"
#include "Subpass.h"
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
		createInstance();				
		createSurface();				
		createDevice();				
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
		createUniformDescriptorSets();
		createInputDescriptorSets();
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

void VulkanRenderer::updateModel(int modelId, glm::mat4& newModel)
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

void VulkanRenderer::updateCameraView(glm::mat4& newView)
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

	/*vkDestroyPipeline(mDevice->logicalDevice(), secondPipeline, nullptr);
	vkDestroyPipelineLayout(mDevice->logicalDevice(), secondPipelineLayout, nullptr);
	vkDestroyPipeline(mDevice->logicalDevice(), graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(mDevice->logicalDevice(), pipelineLayout, nullptr);*/
}

VulkanRenderer::~VulkanRenderer()
{
	if (mDevice)
	{
		mDevice->waitIdle();
	}

	modelList.clear();

	//vkDestroySurfaceKHR(mInstance->handle(), mSurface, nullptr);
	//vkDestroyInstance(mInstance, nullptr);
}

void VulkanRenderer::setupThreadPool()
{
	mThreadCount = std::thread::hardware_concurrency();
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
	// Attachment 0 = Swapchain
	// Attachment 1 = Colour
	// Attachment 2 = Depth

	// CREATE SUBPASS OBJECTS
	std::unique_ptr<Subpass> firstPass = std::make_unique<Subpass>("Shaders/vert.spv", "Shaders/vert.spv");
	std::unique_ptr<Subpass> secondPass = std::make_unique<Subpass>("Shaders/second_vert.spv", "Shaders/second_vert.spv");

	std::vector<uint32_t> outputAttachments = { 1, 2 };

	firstPass->setOutputAttachments(outputAttachments);

	std::vector<uint32_t> inputAttachments = { 1, 2 };

	secondPass->setInputAttachments(inputAttachments);

	mSubpasses.push_back(std::move(firstPass));
	mSubpasses.push_back(std::move(secondPass));

	// CREATE SUBPASS INFOS
	std::vector<SubpassInfo> subpassInfos = {};
	
	// First Subpass
	subpassInfos.push_back({ mSubpasses[0]->inputAttachments(), mSubpasses[0]->outputAttachments() });
	// Second Subpass
	subpassInfos.push_back({ mSubpasses[1]->inputAttachments(), mSubpasses[1]->outputAttachments() });

	// CREATE LOAD/STORE INFOS
	std::vector<LoadStoreInfo> loadStoreInfos = {};

	// swapchain load/store
	loadStoreInfos.push_back({ VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE });
	// Colour load/store
	loadStoreInfos.push_back({ VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE });
	// Depth load/store
	loadStoreInfos.push_back({ VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE });

	// CREATE RENDERPASS OBJECT
	mRenderPass = std::make_unique<RenderPass>(*mDevice,
		mFrames.back()->renderTarget().attachments(),
		subpassInfos,
		loadStoreInfos);

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
	mPushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;	// Shader stage push constant will go to
	mPushConstantRange.offset = 0;								// Offset into given data to pass to push constant
	mPushConstantRange.size = sizeof(glm::mat4);					// Size of data being passed
}

void VulkanRenderer::createGraphicsPipeline()
{
	// PIPELINE 1
	// CREATE SHADER MODULES
	std::vector<ShaderModule> shaderModules;

	std::vector<char> vertexCode = readFile("Shaders/vert.spv");
	shaderModules.emplace_back(*mDevice,
		vertexCode,
		VK_SHADER_STAGE_VERTEX_BIT);

	std::vector<char> fragCode = readFile("Shaders/frag.spv");
	ShaderModule fragShader(*mDevice,
		fragCode,
		VK_SHADER_STAGE_FRAGMENT_BIT);

	shaderModules.push_back(std::move(fragShader));

	/*shaderModules.emplace_back(*mDevice,
		fragCode,
		VK_SHADER_STAGE_FRAGMENT_BIT);*/

	// CREATE PIPELINE LAYOUT
	std::vector<std::reference_wrapper<DescriptorSetLayout>> descriptorSetLayouts = { *mUniformSetLayout , *mSamplerSetLayout };

	std::unique_ptr<PipelineLayout> firstLayout = std::make_unique<PipelineLayout>(*mDevice, descriptorSetLayouts, mPushConstantRange);
	
	// CREATE PIPELINE
	std::unique_ptr<Pipeline> firstPipeline = 
		std::make_unique<GraphicsPipeline>(*mDevice,
			shaderModules,
			*mSwapchain,
			*firstLayout,
			*mRenderPass,
			0,
			VK_TRUE,
			VK_TRUE);

	// Store pipeline + layout
	mPipelineLayouts.push_back(std::move(firstLayout));
	mPipelines.push_back(std::move(firstPipeline));


	// PIPELINE 2
	// CREATE SHADER MODULES
	std::vector<ShaderModule> secondShaderModules = {};

	vertexCode = readFile("Shaders/second_vert.spv");
	secondShaderModules.emplace_back(*mDevice,
		vertexCode,
		VK_SHADER_STAGE_VERTEX_BIT);

	fragCode = readFile("Shaders/second_frag.spv");
	secondShaderModules.emplace_back(*mDevice,
		fragCode,
		VK_SHADER_STAGE_FRAGMENT_BIT);

	// CREATE PIPELINE LAYOUT
	std::vector<std::reference_wrapper<DescriptorSetLayout>> secondDescriptorSetLayouts = { *mAttachmentSetLayout };

	std::unique_ptr<PipelineLayout> secondLayout = std::make_unique<PipelineLayout>(*mDevice, secondDescriptorSetLayouts);

	// CREATE PIPELINE
	std::unique_ptr<Pipeline> secondPipeline =
		std::make_unique<GraphicsPipeline>(*mDevice,
			secondShaderModules,
			*mSwapchain,
			*secondLayout,
			*mRenderPass,
			1,
			VK_FALSE,
			VK_FALSE);

	// Store pipeline + layout
	mPipelineLayouts.push_back(std::move(secondLayout));
	mPipelines.push_back(std::move(secondPipeline));
}

void VulkanRenderer::createFramebuffers()
{
	for (auto& frame : mFrames)
	{
		auto& renderTarget = frame->renderTarget();

		mFramebuffers.push_back(std::make_unique<Framebuffer>(*mDevice, *mRenderPass, renderTarget));
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

	// TODO @ update
	primaryCmdBuffer.beginRenderPass(frame.renderTarget(),
		*mRenderPass,
		*framebuffer,
		clearValues,
		VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	// Split objects to draw equally between threads
	uint32_t numModels = modelList.size();

	float avgObjectsPerBuffer = static_cast<float>(numModels) / mThreadCount;
	uint32_t remainderObjects;
	

	uint32_t objectsPerBuffer  = static_cast<uint32_t> (std::floor(avgObjectsPerBuffer));
	if (objectsPerBuffer == 0)
	{
		remainderObjects = numModels;
	} 
	else
	{
		remainderObjects = numModels % objectsPerBuffer;
	}
		
	
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

	primaryCmdBuffer.bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, *mPipelines[1]);

	std::vector<std::reference_wrapper<const DescriptorSet>> descriptorGroup{ *mAttachmentDescriptorSets[currentImage] };

	primaryCmdBuffer.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, *mPipelineLayouts[1],
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

	cmdBuffer.bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, *mPipelines[0]);

	for (uint32_t i = objectStart; i < objectEnd; ++i)
	{
		MeshModel& thisModel = modelList[i];

		// "Push" constants to given shader stage directly
		cmdBuffer.pushConstant(*mPipelineLayouts[0],
			VK_SHADER_STAGE_VERTEX_BIT,
			thisModel.modelMatrix());


		for (size_t j = 0; j < thisModel.meshCount(); ++j)
		{
			Mesh& thisMesh = thisModel.mesh(j);

			std::vector<std::reference_wrapper<const Buffer>> vertexBuffers{ thisMesh.vertexBuffer() };	// Buffers to bind
			std::vector<VkDeviceSize> offsets{ 0 };															// Offsets into buffers being bound
			cmdBuffer.bindVertexBuffers(0, vertexBuffers, offsets);

			cmdBuffer.bindIndexBuffer(thisMesh.indexBuffer(), 0, VK_INDEX_TYPE_UINT32);

			std::vector<std::reference_wrapper<const DescriptorSet>> descriptorSetGroup{ *mUniformDescriptorSets[activeFrameIndex],
				*mTextureDescriptorSets[thisMesh.texId()] };

			cmdBuffer.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, *mPipelineLayouts[0],
				0, descriptorSetGroup);

			// Execute pipeline
			cmdBuffer.drawIndexed(thisMesh.indexCount(), 1, 0, 0, 0);
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

int VulkanRenderer::createModel(std::string modelFile)
{
	// Import model "scene"
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(modelFile, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

	if (!scene)
	{
		throw std::runtime_error("Failed to load model! (" + modelFile + ")");
	}

	// Get vector of all materials with 1:1 ID placement
	std::vector<std::string> textureNames = LoadMaterials(scene);

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
	std::vector<std::unique_ptr<Mesh>> modelMeshes = LoadNode(*mDevice, scene->mRootNode, scene, matToTex);

	modelList.emplace_back(modelMeshes);

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



