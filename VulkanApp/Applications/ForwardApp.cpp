#include "ForwardApp.h"

ForwardApp::~ForwardApp()
{
	if (mDevice)
	{
		mDevice->waitIdle();
	}
}

void ForwardApp::draw()
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

	updatePerFrameResources();

	// Wait until idle then reset command pools + synchronisation objects
	activeFrame->wait();
	activeFrame->reset();

	// Request the required synchronisation objects
	VkSemaphore renderFinished = activeFrame->requestSemaphore();
	VkFence drawFence = activeFrame->requestFence();

	const Queue& queue = mDevice->queue(mGraphicsQueueFamily, 0);

	CommandBuffer& primaryCmdBuffer = activeFrame->requestCommandBuffer(queue, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	recordCommands(primaryCmdBuffer);		// Only record commands once the image at imageIndex is available (not being used by the queue)


	queue.submit(imageAcquired, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, renderFinished,
		primaryCmdBuffer, drawFence);

	queue.present(renderFinished, *mSwapchain, activeFrameIndex);
}

// Prepare rendertargets and frames
void ForwardApp::createRenderTargetAndFrames()
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
		mColourAttachmentIndex = 1;
		Image colourImage(*mDevice,
			swapchainExtent,
			mColourFormat,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// 2 - depth image
		mDepthAttachmentIndex = 2;
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
	}
}

void ForwardApp::createRenderPass()
{
	// Attachment 0 = Swapchain
	// Attachment 1 = Colour
	// Attachment 2 = Depth

	// CREATE SUBPASS OBJECTS
	std::unique_ptr<Subpass> firstPass = std::make_unique<Subpass>("Shaders/ForwardApp/vert.spv", "Shaders/ForwardApp/frag.spv");
	std::unique_ptr<Subpass> secondPass = std::make_unique<Subpass>("Shaders/Common/fullscreen_vert.spv", "Shaders/ForwardApp/second_frag.spv");

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

// Create layouts which will be updated at most once per frame
// A layout must be created for each pipeline
void ForwardApp::createPerFrameDescriptorSetLayouts()
{

	// Pipeline 1 
	// UNIFORM BUFFERS

	// VP buffer
	ShaderResource vpBuffer(0,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		1,
		VK_SHADER_STAGE_VERTEX_BIT);

	// Lights buffer
	ShaderResource lightBuffer(1,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		1,
		VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

	std::vector<ShaderResource> uniformResources{vpBuffer, lightBuffer};

	//uniformResources.push_back(lightsBuffer);

	// Pipeline 2
	// INPUT ATTACHMENTS
	ShaderResource depthAttachment(0,
		VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
		1,
		VK_SHADER_STAGE_FRAGMENT_BIT);

	ShaderResource colourAttachment(1,
		VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
		1,
		VK_SHADER_STAGE_FRAGMENT_BIT);

	std::vector<ShaderResource> attachmentResources{ depthAttachment , colourAttachment};

	// Create sets in frame objects
	for (auto& frame : mFrames)
	{
		// NOTE : Per frame descriptor set index should always be 0
		// Layout for Pipeline 1
		frame->createDescriptorSetLayout(uniformResources, 0);

		// Layout for Pipeline 2
		frame->createDescriptorSetLayout(attachmentResources, 1);
	}
}

void ForwardApp::createPipelines()
{
	// PIPELINE 1
	// CREATE SHADER MODULES
	std::vector<ShaderModule> shaderModules;

	std::vector<char> vertexCode = readFile(mSubpasses[0]->vertexShaderSource());
	shaderModules.emplace_back(*mDevice,
		vertexCode,
		VK_SHADER_STAGE_VERTEX_BIT);

	std::vector<char> fragCode = readFile(mSubpasses[0]->fragmentShaderSource());
	shaderModules.emplace_back(*mDevice,
		fragCode,
		VK_SHADER_STAGE_FRAGMENT_BIT);

	// CREATE PIPELINE LAYOUT
	std::vector<std::reference_wrapper<const DescriptorSetLayout>> descriptorSetLayouts = { mFrames[0]->descriptorSetLayout(0) , *mPerMaterialDescriptorSetLayout };

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

	vertexCode = readFile(mSubpasses[1]->vertexShaderSource());
	secondShaderModules.emplace_back(*mDevice,
		vertexCode,
		VK_SHADER_STAGE_VERTEX_BIT);

	fragCode = readFile(mSubpasses[1]->fragmentShaderSource());
	secondShaderModules.emplace_back(*mDevice,
		fragCode,
		VK_SHADER_STAGE_FRAGMENT_BIT);

	// CREATE PIPELINE LAYOUT
	std::vector<std::reference_wrapper<const DescriptorSetLayout>> secondDescriptorSetLayouts = { mFrames[0]->descriptorSetLayout(1) };

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

void ForwardApp::createPerFrameResources()
{
	// CREATE UNIFORM BUFFERS

	// Buffer sizes
	VkDeviceSize vpBufferSize = sizeof(uboVP);
	VkDeviceSize lightBufferSize = sizeof(uboLights);

	// Create uniform buffers
	for (size_t i = 0; i < static_cast<size_t>(mSwapchain->details().imageCount); ++i)
	{
		mVPBufferIndex = mFrames[i]->createBuffer(vpBufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		mLightBufferIndex = mFrames[i]->createBuffer(lightBufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}

	createLights();
}

void ForwardApp::createPerFrameDescriptorSets()
{

	for (size_t i = 0; i < static_cast<size_t>(mSwapchain->imageCount()); ++i)
	{
		// PIPELINE 1
		// - BINDING MAP TO PER FRAME BUFFERS
		BindingMap<uint32_t> bufferIndices;
		bufferIndices[0][0] = mVPBufferIndex;
		bufferIndices[1][0] = mLightBufferIndex;

		// - DESCRIPTOR SET
		mFrames[i]->createDescriptorSet(0, {}, bufferIndices);

		// PIPELINE 2
		// - BINDING MAP TO RENDERTARGET IMAGE INDICES
		BindingMap<uint32_t> imageIndices;
		imageIndices[0][0] = mColourAttachmentIndex;
		imageIndices[1][0] = mDepthAttachmentIndex;

		// - DESCRIPTOR SET
		mFrames[i]->createDescriptorSet(1, imageIndices, {});
	}
}

void ForwardApp::updatePerFrameResources()
{
	// Update time variables
	float now = glfwGetTime();
	sumTime += now - lastTime;
	lastTime = now;

	// Set light values
	// - Point lights
	mLights.pointLights[0].position.x = 100 * sin(sumTime);
	mLights.pointLights[2].position.x = -100 * sin(sumTime);

	// - Flash light
	glm::mat4 invView = glm::transpose(mCameraMatrices.V);
	mLights.flashLight.position = invView * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);	// Flash light position is camera position

	// Update buffers
	mFrames[activeFrameIndex]->updateBuffer(mVPBufferIndex, mCameraMatrices);
	mFrames[activeFrameIndex]->updateBuffer(mLightBufferIndex, mLights);
}

// Set required extensions + features
void ForwardApp::getRequiredExtenstionAndFeatures(std::vector<const char*>& requiredExtensions, VkPhysicalDeviceFeatures& requiredFeatures)
{
	requiredExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	requiredFeatures = {};
	requiredFeatures.samplerAnisotropy = VK_TRUE;
}

void ForwardApp::createLights()
{
	// Point Light identical starting positions + intensity
	for (auto& pointLight : mLights.pointLights)
	{
		// Position + intensity
		pointLight.position.x = 0.0f;
		pointLight.position.y = 30.0f;
		pointLight.position.z = 0.0f;
		pointLight.intensityAndAttenuation.x = 50.0f;

		// Attenuation constants
		pointLight.intensityAndAttenuation.y = 0.07f;	// Kq
		pointLight.intensityAndAttenuation.z = 0.14f;	// Kl
		pointLight.intensityAndAttenuation.w = 1.0f;	// Kc
	}
	// Colours
	mLights.pointLights[0].colour = glm::vec4(1.0, 0.5, 0.5, 1.0);
	mLights.pointLights[1].colour = glm::vec4(0.5, 1.0, 0.5, 1.0);
	mLights.pointLights[2].colour = glm::vec4(0.5, 0.5, 1.0, 1.0);

	// Flash Light
	// Position and direction always the same since can be taken from view space
	// e.g. position will always be (0,0,0) and direction will be (0,0,-1)
	mLights.flashLight.position = glm::vec4(0.0);
	mLights.flashLight.direction = glm::vec4(0.0, 0.0, -1.0, 0.0);
	mLights.flashLight.intensityAndAttenuation = glm::vec4(75.0f, 0.07f, 0.14f, 1.0f);
	mLights.flashLight.colour = glm::vec4(1.0);

	mLights.flashLight.innerCutOff = cos(glm::radians(20.0f)); // This should set an overall cone of 40 degrees
	mLights.flashLight.outerCutOff = cos(glm::radians(30.0f)); // This should set an overall cone of 60 degrees

}

void ForwardApp::recordCommands(CommandBuffer& primaryCmdBuffer) // Current image is swapchain index
{
	auto& frame = mFrames[activeFrameIndex];
	auto& framebuffer = mFramebuffers[activeFrameIndex];

	primaryCmdBuffer.beginRecording();

	std::vector<VkClearValue> clearValues;
	clearValues.resize(frame->renderTarget().imageViews().size());
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };				// Clear values for swapchain image (colour)
	clearValues[1].color = { 1.0f, 1.0f, 1.0f, 1.0f };				// Clear values for attachment 1 (colour)
	clearValues[2].depthStencil.depth = 1.0f;						// Clear values for attachment 2 (depth)

	// TODO : update renderpass function
	primaryCmdBuffer.beginRenderPass(frame->renderTarget(),
		*mRenderPass,
		*framebuffer,
		clearValues,
		VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	// Split meshes to draw equally between threads

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

	std::vector<std::reference_wrapper<const DescriptorSet>> descriptorGroup{ frame->descriptorSet(1) };

	primaryCmdBuffer.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, *mPipelineLayouts[1],
		0, descriptorGroup);

	primaryCmdBuffer.draw(3, 1, 0, 0);

	// End Render Pass
	primaryCmdBuffer.endRenderPass();

	// Stop recording to primary command buffers
	primaryCmdBuffer.endRecording();

}


CommandBuffer* ForwardApp::recordSecondaryCommandBuffers(CommandBuffer* primaryCommandBuffer, std::vector<std::reference_wrapper<Mesh>> meshList, uint32_t meshStart, uint32_t meshEnd, size_t threadIndex)
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

		std::vector<std::reference_wrapper<const DescriptorSet>> descriptorSetGroup{ frame->descriptorSet(0, threadIndex),
			*mPerMaterialDescriptorSets[thisMesh.materialID()] };

		cmdBuffer.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, *mPipelineLayouts[0],
			0, descriptorSetGroup);

		// Execute pipeline
		cmdBuffer.drawIndexed(thisMesh.indexCount(), 1, 0, 0, 0);
	}

	// Stop recording to primary command buffers
	cmdBuffer.endRecording();

	return &cmdBuffer;

}