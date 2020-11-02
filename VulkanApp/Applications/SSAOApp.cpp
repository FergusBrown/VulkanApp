#include "SSAOApp.h"
#include <random>

SSAOApp::~SSAOApp()
{
	if (mDevice)
	{
		mDevice->waitIdle();
	}
}

void SSAOApp::draw()
{
	auto& previousFrame = mFrames[activeFrameIndex];

	// Get next active frame
	VkSemaphore imageAcquired = previousFrame->requestSemaphore();	// Get a semaphore from the pool of the previous frame
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
void SSAOApp::createRenderTargetAndFrames()
{
	//auto& device = mSwapchain->device();
	auto& swapchainExtent = mSwapchain->extent();
	VkFormat swapchainFormat = mSwapchain->format();
	VkImageUsageFlags swapchainUsage = mSwapchain->usage();

	// Get supported format for position attachment - use higher precision floats
	mPrecisionFormat = VK_FORMAT_R32G32B32A32_SFLOAT;

	// High precision not required
	mColourFormat = VK_FORMAT_R8G8B8A8_UNORM;

	// Get supported format for depth buffer
	mDepthFormat = chooseSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	// Requires to store a single float
	mSSAOFormat = VK_FORMAT_R8_UNORM;

	// NOTE ON IMAGE USAGE:
	// VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT - always use if image will be stored and sampled/loaded in a future subpass
	// VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT - use if attachment will be referenced as a subpass input in renderpass creation
	// VK_IMAGE_USAGE_SAMPLED_BIT - use if image will be sampled

	for (auto& image : mSwapchain->images())
	{
		// IMAGES + RENDERTARGET + RESOURCE REFERENCE

		// SUBPASS 3 OUTPUT (LIGHTING)
		// 0 - swapchain image
		Image swapchainImage(*mDevice,
			image,
			swapchainExtent,
			swapchainFormat,
			swapchainUsage);

		// SUBPASS 2 OUTPUT (BLUR PASS)
		// 1 - Blur
		mBlurAttachmentIndex = 1;
		Image blurImage(*mDevice,
			swapchainExtent,
			//mSSAOFormat,
			mColourFormat,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// SUBPASS 1 OUTPUT (SSAO PASS)
		// 2 - SSAO
		mSSAOAttachmentIndex = 2;
		Image ssaoImage(*mDevice,
			swapchainExtent,
			//mSSAOFormat,
			mColourFormat,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// SUBPASS 0 OUTPUT (GEOMETRY PASS)
		// 3 - Position
		mPositionAttachmentIndex = 3;
		Image positionImage(*mDevice,
			swapchainExtent,
			mPrecisionFormat,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// 4 - Normals
		mNormalAttachmentIndex = 4;
		Image normalImage(*mDevice,
			swapchainExtent,
			mColourFormat,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// 5 - Albedo 
		mAlbedoAttachmentIndex = 5;
		Image albedoImage(*mDevice,
			swapchainExtent,
			mColourFormat,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// 6 - Specular
		mSpecularAttachmentIndex = 6;
		Image specularImage(*mDevice,
			swapchainExtent,
			mColourFormat,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// 7 - Depth
		mDepthAttachmentIndex = 7;
		Image depthImage(*mDevice,
			swapchainExtent,
			mDepthFormat,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


		std::vector<Image> renderTargetImages;
		renderTargetImages.push_back(std::move(swapchainImage));
		renderTargetImages.push_back(std::move(blurImage));
		renderTargetImages.push_back(std::move(ssaoImage));
		renderTargetImages.push_back(std::move(positionImage));
		renderTargetImages.push_back(std::move(normalImage));
		renderTargetImages.push_back(std::move(albedoImage));
		renderTargetImages.push_back(std::move(specularImage));
		renderTargetImages.push_back(std::move(depthImage));

		// Create Render Target + Frame
		std::unique_ptr<RenderTarget> renderTarget = std::make_unique<RenderTarget>(std::move(renderTargetImages));
		mFrames.push_back(std::make_unique<Frame>(*mDevice, std::move(renderTarget), mThreadCount));
	}
}

void SSAOApp::createRenderPass()
{
	// Attachment 0 = Swapchain
	// Attachment 1 = Blur
	// Attachment 2 = SSAO
	// Attachment 3 = Position
	// Attachment 4 = Normal
	// Attachment 5 = Albedo
	// Attachment 6 = Specular
	// Attachment 7 = Depth

	// CREATE SUBPASS OBJECTS
	uint32_t subpassCount = 4;
	mSubpasses.resize(subpassCount);
	mSubpasses[0] = std::make_unique<Subpass>("Shaders/SSAOApp/geometry_vert.spv", "Shaders/SSAOApp/geometry_frag.spv");
	mSubpasses[1] =	std::make_unique<Subpass>("Shaders/Common/fullscreen_vert.spv", "Shaders/SSAOApp/ssao_frag.spv");
	mSubpasses[2] =	std::make_unique<Subpass>("Shaders/Common/fullscreen_vert.spv", "Shaders/SSAOApp/blur_frag.spv");
	mSubpasses[3] = std::make_unique<Subpass>("Shaders/Common/fullscreen_vert.spv", "Shaders/SSAOApp/lighting_frag.spv");

	// Set input and output attachments
	// Note that these inputs and outputs are not used to define when an attachment will be used for a subpass load
	// They are used to inform the renderpass creation of what layout each attachment image should have at each subpass so that the necessary image transitions can take place
	// Therefore, even if an image is sampled rather than subpass loaded it should be listed as an input
	std::vector<uint32_t> inputAttachments{};
	std::vector<uint32_t> outputAttachments{};

	// SUBPASS 0 (GEOMETRY)
	outputAttachments = { mPositionAttachmentIndex, mNormalAttachmentIndex, mAlbedoAttachmentIndex, mSpecularAttachmentIndex, mDepthAttachmentIndex };
	mSubpasses[0]->setOutputAttachments(outputAttachments);

	// SUBPASS 1 (SSAO)
	inputAttachments = { mPositionAttachmentIndex, mNormalAttachmentIndex };
	outputAttachments = { mSSAOAttachmentIndex };
	mSubpasses[1]->setInputAttachments(inputAttachments);
	mSubpasses[1]->setOutputAttachments(outputAttachments);

	// SUBPASS 2 (BLUR)
	inputAttachments = outputAttachments;
	outputAttachments = { mBlurAttachmentIndex };
	mSubpasses[2]->setInputAttachments(inputAttachments);
	mSubpasses[2]->setOutputAttachments(outputAttachments);

	// SUBPASS 3 (LIGHTING)
	inputAttachments = { mPositionAttachmentIndex, mNormalAttachmentIndex, mAlbedoAttachmentIndex, mSpecularAttachmentIndex, mDepthAttachmentIndex, mBlurAttachmentIndex };
	mSubpasses[3]->setInputAttachments(inputAttachments);

	// CREATE SUBPASS INFOS
	std::vector<SubpassInfo> subpassInfos(subpassCount);

	for (uint32_t i = 0; i < subpassCount; ++i)
	{
		subpassInfos[i] = { mSubpasses[i]->inputAttachments(), mSubpasses[i]->outputAttachments() };
	}

	// CREATE LOAD/STORE INFOS FOR ATTACHMENTS
	// LOAD_OP - what to do with the image at the start of the renderpass
	// STORE_OP - what to do with the image at the end of the renderpass
	std::vector<LoadStoreInfo> loadStoreInfos = {};

	// Swapchain load/store
	loadStoreInfos.push_back({ VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE });

	size_t attachmentCount = mFrames[0]->renderTarget().attachments().size();

	// Remaining load stores should all be discarded after renderpass is complete
	for (size_t i = 0; i < attachmentCount - 1; ++i)
	{
		loadStoreInfos.push_back({ VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE });
	}

	// CREATE RENDERPASS OBJECT
	mRenderPass = std::make_unique<RenderPass>(*mDevice,
		mFrames.back()->renderTarget().attachments(),
		subpassInfos,
		loadStoreInfos);
}

// Create layouts which will be updated at most once per frame
// A layout must be created for each pipeline
void SSAOApp::createPerFrameDescriptorSetLayouts()
{
	std::vector<std::vector<ShaderResource>> pipelineResources(mSubpasses.size());

	// BIND BUFFERS AND SAMPLERS
	// Pipeline 0 - No input attachments, remaining bindings start at index 0
	// VP buffer
	ShaderResource vpBuffer(0,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		1,
		VK_SHADER_STAGE_VERTEX_BIT);

	pipelineResources[0].push_back(std::move(vpBuffer));

	// Pipeline 1
	// Position sampler
	ShaderResource positionSampler(0,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		1,
		VK_SHADER_STAGE_FRAGMENT_BIT);
	pipelineResources[1].push_back(std::move(positionSampler));

	// Normal sampler
	ShaderResource normalSampler(1,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		1,
		VK_SHADER_STAGE_FRAGMENT_BIT);
	pipelineResources[1].push_back(std::move(normalSampler));

	// Noise sampler
	ShaderResource noiseSampler(2,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		1,
		VK_SHADER_STAGE_FRAGMENT_BIT);
	pipelineResources[1].push_back(std::move(noiseSampler));

	// VP buffer
	ShaderResource vBuffer(3,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		1,
		VK_SHADER_STAGE_FRAGMENT_BIT);
	pipelineResources[1].push_back(std::move(vBuffer));

	// SSAO Buffer
	ShaderResource ssaoBuffer(4,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		1,
		VK_SHADER_STAGE_FRAGMENT_BIT);
	pipelineResources[1].push_back(std::move(ssaoBuffer));

	// Pipeline 2
	// SSAO input sampler
	ShaderResource ssaoSampler(0,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		1,
		VK_SHADER_STAGE_FRAGMENT_BIT);
	pipelineResources[2].push_back(std::move(ssaoSampler));

	// Pipeline 3
	// BIND INPUT ATTACHMENTS
	// For each pipeline define input attachment shader resources
	for (uint32_t i = 0; i < mSubpasses[3]->inputAttachments().size(); ++i)
	{
		ShaderResource attachment(i,
			VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT);

		pipelineResources[3].push_back(std::move(attachment));
	}

	// Lights buffer
	ShaderResource lightBuffer(6,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		1,
		VK_SHADER_STAGE_FRAGMENT_BIT);
	pipelineResources[3].push_back(std::move(lightBuffer));

	// Create set layouts in frame objects for each pipeline
	for (auto& frame : mFrames)
	{
		for (uint32_t i = 0; i < pipelineResources.size(); ++i)
		{
			frame->createDescriptorSetLayout(pipelineResources[i], i);
		}
	}
}

void SSAOApp::createPipelines()
{
	mPipelineLayouts.resize(mSubpasses.size());
	mPipelines.resize(mSubpasses.size());
	std::vector<std::vector<ShaderModule>> shaderModules(mSubpasses.size());

	// Create shader modules
	for (size_t i = 0; i < shaderModules.size(); ++i)
	{
		std::vector<char> vertexCode = readFile(mSubpasses[i]->vertexShaderSource());
		shaderModules[i].emplace_back(*mDevice,
			vertexCode,
			VK_SHADER_STAGE_VERTEX_BIT);

		std::vector<char> fragCode = readFile(mSubpasses[i]->fragmentShaderSource());
		shaderModules[i].emplace_back(*mDevice,
			fragCode,
			VK_SHADER_STAGE_FRAGMENT_BIT);
	}

	// PIPELINE 0 - this pipeline has the addition layout for per material descriptors and also requires a push constant range due to use of push constants
	// CREATE PIPELINE LAYOUT
	std::vector<std::reference_wrapper<const DescriptorSetLayout>> descriptorSetLayouts = { mFrames[0]->descriptorSetLayout(0) , *mPerMaterialDescriptorSetLayout };

	mPipelineLayouts[0] = std::make_unique<PipelineLayout>(*mDevice, descriptorSetLayouts, mPushConstantRange);

	// CREATE PIPELINE
	mPipelines[0] = std::make_unique<GraphicsPipeline>(*mDevice,
		shaderModules[0],
		*mSwapchain,
		*mPipelineLayouts[0],
		*mRenderPass,
		0,
		VK_TRUE,
		VK_TRUE);

	// Remaining pipelines can be generated in the same fashion as they all draw their descriptor set layout from the frame objects
	for (size_t i = 1; i < shaderModules.size(); ++i)
	{
		uint32_t subpassIndex = static_cast<uint32_t>(i);

		// CREATE PIPELINE LAYOUT
		descriptorSetLayouts = { mFrames[0]->descriptorSetLayout(subpassIndex) };

		mPipelineLayouts[i] = std::make_unique<PipelineLayout>(*mDevice, descriptorSetLayouts);

		// CREATE PIPELINE
		mPipelines[i] = std::make_unique<GraphicsPipeline>(*mDevice,
			shaderModules[i],
			*mSwapchain,
			*mPipelineLayouts[i],
			*mRenderPass,
			subpassIndex,
			VK_FALSE,
			VK_FALSE);
	}
}

void SSAOApp::createPerFrameResources()
{
	// CREATE UNIFORM BUFFERS
	// Buffer sizes
	VkDeviceSize vpBufferSize = sizeof(uboVP);
	VkDeviceSize lightBufferSize = sizeof(uboLights);

	// Create VP and lighting buffers
	for (size_t i = 0; i < mFrames.size(); ++i)
	{
		mVPBufferIndex = mFrames[i]->createBuffer(vpBufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		mLightBufferIndex = mFrames[i]->createBuffer(lightBufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}

	// Lights
	createLights();

	// SSAO kernel + noise texture
	createSSAOResources();

	// Samplers for sampling attachments
	// (Only position at the moment)
	createAttachmentSamplers();
}

void SSAOApp::createPerFrameDescriptorSets()
{
	for (size_t i = 0; i < mFrames.size(); ++i)
	{
		// Bind image and buffers to resource reference
		DescriptorResourceReference descriptorSetResourceReference;
		BindingMap<uint32_t> bufferIndices;

		// Get render target images to create sampler desctiptors
		auto& targetImages = mFrames[i]->renderTarget().imageViews();

		//************************************

		// PIPELINE 0 (Lighting)
		// - RESOURCE REFERENCES
		// VP buffer
		bufferIndices[0][0] = mVPBufferIndex;

		// - DESCRIPTOR SET
		mFrames[i]->createDescriptorSet(0, descriptorSetResourceReference, bufferIndices);

		descriptorSetResourceReference.reset();
		bufferIndices.clear();

		// PIPELINE 1
		// - RESOURCE REFERENCES
		// Position sampler
		descriptorSetResourceReference.bindImage(targetImages[mPositionAttachmentIndex], *mPositionSampler, 0, 0);

		// Normal sampler
		descriptorSetResourceReference.bindImage(targetImages[mNormalAttachmentIndex], *mNormalSampler, 1, 0);

		// Noise sampler
		descriptorSetResourceReference.bindImage(mNoiseTexture->imageView(), *mNormalSampler, 2, 0);

		// VP buffer
		bufferIndices[3][0] = mVPBufferIndex;

		// SSAO kernel buffer
		bufferIndices[4][0] = mSSAOBufferIndex;

		// - DESCRIPTOR SET
		mFrames[i]->createDescriptorSet(1, descriptorSetResourceReference, bufferIndices);

		descriptorSetResourceReference.reset();
		bufferIndices.clear();

		// PIPELINE 2
		// - RESOURCE REFERENCES
		// Input attachments
		descriptorSetResourceReference.bindImage(targetImages[mSSAOAttachmentIndex], *mSSAOSampler, 0, 0);

		// - DESCRIPTOR SET
		mFrames[i]->createDescriptorSet(2, descriptorSetResourceReference, bufferIndices);

		descriptorSetResourceReference.reset();
		bufferIndices.clear();

		// PIPELINE 3 (Lighting)
		// - BINDING MAP
		descriptorSetResourceReference.bindInputImage(targetImages[mPositionAttachmentIndex], 0, 0);
		descriptorSetResourceReference.bindInputImage(targetImages[mNormalAttachmentIndex], 1, 0);
		descriptorSetResourceReference.bindInputImage(targetImages[mAlbedoAttachmentIndex], 2, 0);
		descriptorSetResourceReference.bindInputImage(targetImages[mSpecularAttachmentIndex], 3, 0);
		descriptorSetResourceReference.bindInputImage(targetImages[mDepthAttachmentIndex], 4, 0);
		descriptorSetResourceReference.bindInputImage(targetImages[mBlurAttachmentIndex], 5, 0);

		bufferIndices[6][0] = mLightBufferIndex;

		// - DESCRIPTOR SET
		mFrames[i]->createDescriptorSet(3, descriptorSetResourceReference, bufferIndices);

	}
}

void SSAOApp::createSSAOResources()
{
	std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
	std::default_random_engine generator;

	// KERNEL
	uboSSAO ssaoBuffer;
	auto& kernel = ssaoBuffer.ssaoKernel;
	for (size_t i = 0; i < SAMPLE_COUNT; ++i)
	{
		// x and y in rnage of [-1,1]
		// z in range of [0,1] creating a hemisphere of samples rather than sphere
		glm::vec4 sample(
			distribution(generator) * 2.0f - 1,
			distribution(generator) * 2.0f - 1,
			distribution(generator),
			0.0);	// 4th element obsolete but needed for padding the UBO struct

		sample = glm::normalize(sample);
		sample *= distribution(generator);

		// Use accelerating interpolation so that samples are weighted towards the centre of the hemisphere
		float scale = (float)i / SAMPLE_COUNT;
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;

		kernel[i] = sample;
	}

	// Create buffers to store kernel data and upload them to buffer
	VkDeviceSize bufferSize = sizeof(uboSSAO);
	for (size_t i = 0; i < mFrames.size(); ++i)
	{
		mSSAOBufferIndex = mFrames[i]->createBuffer(bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		mFrames[i]->updateBuffer(mSSAOBufferIndex, ssaoBuffer);
	}

	// NOISE TEXTURE
	// Create 4x4 noise texture values - this will hold a series of random rotation vectors around the z axis
	// vector will hold the 16 values in sequence
	std::vector<glm::vec4> ssaoNoise;		
	uint32_t width = 4;
	uint32_t height = 4;
	for (uint32_t i = 0; i < width * height; ++i)
	{
		// Z axis left as 0 as rotation is around this axis
		glm::vec4 noise(
			distribution(generator) * 2.0f - 1,
			distribution(generator) * 2.0f - 1,
			0.0f,
			0.0f);

		ssaoNoise.push_back(std::move(noise));
	}

	// Create the texture - must be stored as float
	VkDeviceSize textureSize = ssaoNoise.size() * sizeof(glm::vec4);
	VkFormat noiseFormat = VK_FORMAT_R32G32B32A32_SFLOAT;	// Note that vector type must match image format - e.g. this format is stored as vec4
	mNoiseTexture = std::make_unique<Texture>(*mDevice, ssaoNoise.data(), width, height, textureSize, noiseFormat);
	
	// NOISE TEXTURE SAMPLER
	// Ensure that noise sampler is set to VK_SAMPLER_ADDRESS_MODE_REPEAT so that the texture is tiled across the screen
	mNoiseSampler = std::make_unique<Sampler>(
		*mDevice,
		VK_FALSE);		// Anisotropic filtering not required
}

void SSAOApp::createAttachmentSamplers()
{
	// Position , normal and SSAO samplers
	// This attachment requires clamp to edge to ensure that position/depth valuesa are not oversampled outside of default texture coords
	mPositionSampler = std::make_unique<Sampler>(
		*mDevice,
		VK_FALSE,
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		VK_FILTER_LINEAR,
		VK_FILTER_LINEAR,
		VK_SAMPLER_MIPMAP_MODE_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);		

	mNormalSampler = std::make_unique<Sampler>(
		*mDevice,
		VK_FALSE,
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		VK_FILTER_LINEAR,
		VK_FILTER_LINEAR,
		VK_SAMPLER_MIPMAP_MODE_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

	mSSAOSampler = std::make_unique<Sampler>(
		*mDevice,
		VK_FALSE,
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		VK_FILTER_LINEAR,
		VK_FILTER_LINEAR,
		VK_SAMPLER_MIPMAP_MODE_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

}


void SSAOApp::updatePerFrameResources()
{
	// Update time variables
	float now = glfwGetTime();
	sumTime += now - lastTime;
	lastTime = now;

	// Set light values
	// - Point lights
	mLights.pointLights[0].position.x = 100 * sin(sumTime);
	mLights.pointLights[1].position.x = 0;
	mLights.pointLights[2].position.x = -100 * sin(sumTime);

	// set yz values
	for (auto& light : mLights.pointLights)
	{
		light.position.y = 30.0f;
		light.position.z = 0.0f;
	}

	// Convert positions to view space
	// Note that flashlight pos and dir are already in view space so no conversion is necessary
	for (auto& light : mLights.pointLights)
	{
		light.position = mCameraMatrices.V * light.position;
	}

	// Update buffers
	mFrames[activeFrameIndex]->updateBuffer(mVPBufferIndex, mCameraMatrices);
	mFrames[activeFrameIndex]->updateBuffer(mLightBufferIndex, mLights);
}

// Set required extensions + features
void SSAOApp::getRequiredExtenstionAndFeatures(std::vector<const char*>& requiredExtensions, VkPhysicalDeviceFeatures& requiredFeatures)
{
	requiredExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	requiredFeatures = {};
	requiredFeatures.samplerAnisotropy = VK_TRUE;
}

void SSAOApp::createLights()
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

void SSAOApp::recordCommands(CommandBuffer& primaryCmdBuffer) // Current image is swapchain index
{
	auto& frame = mFrames[activeFrameIndex];
	auto& framebuffer = mFramebuffers[activeFrameIndex];

	primaryCmdBuffer.beginRecording();

	// Set all clear values
	std::vector<VkClearValue> clearValues;
	clearValues.resize(frame->renderTarget().imageViews().size());

	// All but the last render target are colour attachments
	for (size_t i = 0; i < clearValues.size(); ++i)
	{
		clearValues[i].color = { 0.0f, 0.0f, 0.0f, 1.0f };

	}
	// Final attachment is depth attachment
	clearValues.back().depthStencil.depth = 1.0f;

	//clearValue.depthStencil.depth = 1.0f;
	// BEGIN RENDERPASS / SUBPASS 0
	primaryCmdBuffer.beginRenderPass(frame->renderTarget(),
		*mRenderPass,
		*framebuffer,
		clearValues,
		VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	// Split meshes to draw equally between threads
	// Create vector of references to meshes
	std::vector<std::reference_wrapper<Mesh>> meshList;
	//std::vector<std::reference_wrapper<Mesh>> transparentList;
	for (auto& model : mModelList)
	{
		for (size_t i = 0; i < model.meshCount(); ++i)
		{
			auto& mesh = model.mesh(i);
			//if (mesh.opaque())
			//{
				meshList.push_back(mesh);
			//} 
			//else
			//{
			//	transparentList.push_back(mesh);
			//}
			
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

	// Record remaining subpasses on primary comman buffers
	// All remaining subpass perform fragment shader operations rendered to a full screen triangle
	for (uint32_t i = 1; i < mSubpasses.size(); ++i)
	{
		primaryCmdBuffer.nextSubpass(VK_SUBPASS_CONTENTS_INLINE);

		primaryCmdBuffer.bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, *mPipelines[i]);

		std::vector<std::reference_wrapper<const DescriptorSet>> descriptorGroup{ frame->descriptorSet(i) };

		primaryCmdBuffer.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, *mPipelineLayouts[i],
			0, descriptorGroup);

		primaryCmdBuffer.drawFullscreen();
	}
	
	// End Render Pass
	primaryCmdBuffer.endRenderPass();

	// STOP RECORDING
	primaryCmdBuffer.endRecording();

}

CommandBuffer* SSAOApp::recordSecondaryCommandBuffers(CommandBuffer* primaryCommandBuffer, std::vector<std::reference_wrapper<Mesh>> meshList, uint32_t meshStart, uint32_t meshEnd, size_t threadIndex)
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