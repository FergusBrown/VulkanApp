#include "BasicApp.h"

BasicApp::~BasicApp()
{
	if (mDevice)
	{
		mDevice->waitIdle();
	}
}

void BasicApp::draw()
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

	recordCommands(primaryCmdBuffer);		// Only record commands once the image at imageIndex is available (not being used by the queue)

	updateUniformBuffers();

	queue.submit(imageAcquired, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, renderFinished,
		primaryCmdBuffer, drawFence);

	queue.present(renderFinished, *mSwapchain, activeFrameIndex);
}

// Prepare rendertargets and frames
void BasicApp::createRenderTargetAndFrames()
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

void BasicApp::createRenderPass()
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

void BasicApp::createDescriptorSetLayouts()
{
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

void BasicApp::createPushConstantRange()
{
	mPushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;	// Shader stage push constant will go to
	mPushConstantRange.offset = 0;								// Offset into given data to pass to push constant
	mPushConstantRange.size = sizeof(glm::mat4);					// Size of data being passed
}

void BasicApp::createPipelines()
{
	// PIPELINE 1
	// CREATE SHADER MODULES
	std::vector<ShaderModule> shaderModules;

	std::vector<char> vertexCode = readFile("Shaders/vert.spv");
	shaderModules.emplace_back(*mDevice,
		vertexCode,
		VK_SHADER_STAGE_VERTEX_BIT);

	std::vector<char> fragCode = readFile("Shaders/frag.spv");
	shaderModules.emplace_back(*mDevice,
		fragCode,
		VK_SHADER_STAGE_FRAGMENT_BIT);

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



void BasicApp::createDescriptorPools()
{
	// CREATE INPUT ATTACHMENT DESCRIPTOR POOL
	mAttachmentDescriptorPool = std::make_unique<DescriptorPool>(*mDevice, *mAttachmentSetLayout, mSwapchain->imageCount());
}

void BasicApp::createDescriptorSets()
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

// Set required extensions + features
void BasicApp::getRequiredExtenstionAndFeatures(std::vector<const char*>& requiredExtensions, VkPhysicalDeviceFeatures& requiredFeatures)
{
	requiredExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	requiredFeatures = {};
	requiredFeatures.samplerAnisotropy = VK_TRUE;
}

void BasicApp::recordCommands(CommandBuffer& primaryCmdBuffer) // Current image is swapchain index
{
	auto& frame = *mFrames[activeFrameIndex];
	auto& framebuffer = mFramebuffers[activeFrameIndex];

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

	// Split meshes to draw equally between threads

	// TODO : alter this so that it isn't generated every frame
	// Create vector of references to meshes
	std::vector<std::reference_wrapper<Mesh>> meshList;
	for (auto& model : mModelList)
	{
		for (size_t i = 0; i < model.meshCount(); ++i)
		{
			meshList.push_back(model.mesh(i));
		}
	}

	uint32_t meshCount = static_cast<uint32_t>(meshList.size());

	// TODO : alter this so that it isn't generated every frame
	// Create vector of references to meshes


	float avgMeshesPerBuffer = static_cast<float>(meshCount) / mThreadCount;
	uint32_t remainderMeshes;



	uint32_t meshesPerBuffer = static_cast<uint32_t> (std::floor(avgMeshesPerBuffer));
	if (meshesPerBuffer == 0)
	{
		remainderMeshes = meshCount;
	}
	else
	{
		remainderMeshes = meshCount % meshesPerBuffer;
	}


	uint32_t meshStart = 0;

	// Vector for results of tasks pushed to threadpool
	std::vector<std::future<CommandBuffer*>> futureSecondaryCommandBuffers;

	for (uint32_t i = 0; i < mThreadCount; ++i)
	{
		// Get the end index for the last mesh which will be handled by this buffer
		uint32_t meshEnd = std::min(meshCount, meshStart + meshesPerBuffer);

		// If there are still remainder draws then add a draw to this command buffer
		// Latter command buffers may contain fewer draws
		if (remainderMeshes > 0)
		{
			meshEnd++;
			remainderMeshes--;
		}

		// Push lambda function to threadpool for running
		auto futureResult = mThreadPool.push([=, &primaryCmdBuffer](size_t threadIndex) {
			return recordSecondaryCommandBuffers(&primaryCmdBuffer, meshList, meshStart, meshEnd, threadIndex);
			});

		futureSecondaryCommandBuffers.push_back(std::move(futureResult));

		meshStart = meshEnd;
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

	std::vector<std::reference_wrapper<const DescriptorSet>> descriptorGroup{ *mAttachmentDescriptorSets[activeFrameIndex] };

	primaryCmdBuffer.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, *mPipelineLayouts[1],
		0, descriptorGroup);

	primaryCmdBuffer.draw(3, 1, 0, 0);

	// End Render Pass
	primaryCmdBuffer.endRenderPass();

	// Stop recording to primary command buffers
	primaryCmdBuffer.endRecording();

}


CommandBuffer* BasicApp::recordSecondaryCommandBuffers(CommandBuffer* primaryCommandBuffer, std::vector<std::reference_wrapper<Mesh>> meshList, uint32_t meshStart, uint32_t meshEnd, size_t threadIndex)
{
	auto& frame = mFrames[activeFrameIndex];

	auto& queue = mDevice->queue(mGraphicsQueueFamily, 0);

	CommandBuffer& cmdBuffer = frame->requestCommandBuffer(queue, VK_COMMAND_BUFFER_LEVEL_SECONDARY, threadIndex);

	cmdBuffer.beginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		| VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT, primaryCommandBuffer);

	cmdBuffer.bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, *mPipelines[0]);

	for (uint32_t i = meshStart; i < meshEnd; ++i)
	{
		Mesh& thisMesh = meshList[i];

		// "Push" constants to given shader stage directly
		cmdBuffer.pushConstant(*mPipelineLayouts[0],
			VK_SHADER_STAGE_VERTEX_BIT,
			thisMesh.model());

		std::vector<std::reference_wrapper<const Buffer>> vertexBuffers{ thisMesh.vertexBuffer() };	// Buffers to bind
		std::vector<VkDeviceSize> offsets{ 0 };														// Offsets into buffers being bound
		cmdBuffer.bindVertexBuffers(0, vertexBuffers, offsets);

		cmdBuffer.bindIndexBuffer(thisMesh.indexBuffer(), 0, VK_INDEX_TYPE_UINT32);

		std::vector<std::reference_wrapper<const DescriptorSet>> descriptorSetGroup{ *mUniformDescriptorSets[activeFrameIndex],
			*mTextureDescriptorSets[thisMesh.texId()] };

		cmdBuffer.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, *mPipelineLayouts[0],
			0, descriptorSetGroup);

		// Execute pipeline
		cmdBuffer.drawIndexed(thisMesh.indexCount(), 1, 0, 0, 0);
	}

	// Stop recording to primary command buffers
	cmdBuffer.endRecording();

	return &cmdBuffer;

}