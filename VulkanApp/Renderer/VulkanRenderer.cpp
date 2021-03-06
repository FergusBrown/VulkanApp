#define STB_IMAGE_IMPLEMENTATION
#include "VulkanRenderer.h"

int VulkanRenderer::init(GLFWwindow* newWindow)
{
	mWindow = newWindow;

	try {
		setupThreadPool();

		createInstance();				
		createSurface();				
		createDevice();				
		findDesiredQueueFamilies();

		chooseImageFormats();
		createSwapchain();				
		createRenderTargetAndFrames();
		createRenderPass();	

		createPerFrameDescriptorSetLayouts();
		createPerMaterialDescriptorSetLayout();
		createPushConstantRange();

		createPipelines();
		createFramebuffers();

		createMaterialSamplers();
		createTexture("default_black.png"); // Default texture (if no texture present)
		createPerFrameResources();

		createPerMaterialDescriptorPool();
		
		createPerFrameDescriptorSets();
		createCamera(90.0f);

		

	}
	catch (const std::runtime_error& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void VulkanRenderer::updateModel(int modelId, glm::mat4& newModel)
{
	if (modelId >= mModelList.size()) return;

	mModelList[modelId].setModel(newModel);
}

void VulkanRenderer::createCamera(float FoVinDegrees)
{
	const VkExtent2D& extent = mSwapchain->extent();

	mCameraMatrices.P = glm::perspective(glm::radians(FoVinDegrees), (float)extent.width / (float)extent.height, 0.1f, 300.0f);
	mCameraMatrices.V = glm::lookAt(glm::vec3(0.0f, 0.0f, 20.0f), glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	mCameraMatrices.P[1][1] *= -1;
}

void VulkanRenderer::updateCameraView(glm::mat4& newView)
{
	mCameraMatrices.V = newView;
}

VulkanRenderer::~VulkanRenderer()
{
	if (mDevice)
	{
		mDevice->waitIdle();
	}

	mModelList.clear();
	mTextures.clear();
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
	std::vector<const char*> requiredExtensions = {};

	VkPhysicalDeviceFeatures requiredFeatures = {};

	getRequiredExtenstionAndFeatures(requiredExtensions, requiredFeatures);

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
// Set supported formats as default formats
void VulkanRenderer::chooseImageFormats()
{
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
}

void VulkanRenderer::createPerMaterialDescriptorSetLayout()
{
	// TEXTURE SAMPLERS
	ShaderResource diffuseSampler(0,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		1,				
		VK_SHADER_STAGE_FRAGMENT_BIT);

	ShaderResource normalSampler(1,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		1,
		VK_SHADER_STAGE_FRAGMENT_BIT);

	ShaderResource specularSampler(2,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		1,
		VK_SHADER_STAGE_FRAGMENT_BIT);

	std::vector<ShaderResource> samplerResources{ diffuseSampler, normalSampler, specularSampler };

	mPerMaterialDescriptorSetLayout = (std::make_unique<DescriptorSetLayout>(*mDevice, 1, samplerResources));
}

void VulkanRenderer::createPushConstantRange()
{
	mPushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;		// Shader stage push constant will go to
	mPushConstantRange.offset = 0;									// Offset into given data to pass to push constant
	mPushConstantRange.size = sizeof(glm::mat4);					// Size of data being passed
}


void VulkanRenderer::createFramebuffers()
{
	for (auto& frame : mFrames)
	{
		auto& renderTarget = frame->renderTarget();

		mFramebuffers.push_back(std::make_unique<Framebuffer>(*mDevice, *mRenderPass, renderTarget));
	}
}

void VulkanRenderer::createMaterialSamplers()
{
	float maxAnisotropy = mDevice->physicalDevice().properties().limits.maxSamplerAnisotropy;

	mDiffuseSampler = std::make_unique<Sampler>(*mDevice,
		VK_TRUE,
		maxAnisotropy,
		0.0f,
		MAX_LOD);	// Max LOD can be as high as possible since it is just used to clamp the computed LOD

	mNormalSampler = std::make_unique<Sampler>(*mDevice,
		VK_TRUE,
		maxAnisotropy,
		0.0f,
		MAX_LOD);

	mSpecularSampler = std::make_unique<Sampler>(*mDevice,
		VK_TRUE,
		maxAnisotropy,
		0.0f,
		MAX_LOD);
}


void VulkanRenderer::createPerMaterialDescriptorPool()
{
	// CREATE SAMPLER DESCRIPTOR POOL
	mPerMaterialDescriptorPool = std::make_unique<DescriptorPool>(*mDevice, *mPerMaterialDescriptorSetLayout, MAX_MATERIALS);
}

VkFormat VulkanRenderer::chooseSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags)
{
	// Loop through the options and find a compatible one
	for (VkFormat format : formats)
	{
		// Get properties for given format on this device
		VkFormatProperties properties;
		mDevice->physicalDevice().getFormatProperties(format, properties);

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

uint32_t VulkanRenderer::createTexture(std::string fileName)
{
	// Load in the image file
	int width, height;
	VkDeviceSize imageSize;
	stbi_uc* textureData = loadTextureFile(fileName, width, height, imageSize);

	std::unique_ptr<Texture>  texture = std::make_unique<Texture>(*mDevice, textureData, width, height, imageSize);

	// Add texture to map of textures
	int textureID = texture->textureID();;
	mTextures[textureID] = std::move(texture);
	assert(!texture); // just checking ownership of the texture ptr has moved to the map
	
	return textureID;
}

uint32_t VulkanRenderer::createMaterialDescriptor(uint32_t diffuseID, uint32_t normalID, uint32_t specularID)
{
	// Create descriptor resource reference
	DescriptorResourceReference materialResource;
	// Bind Images
	materialResource.bindImage(mTextures[diffuseID]->imageView(), *mDiffuseSampler, 0, 0);
	materialResource.bindImage(mTextures[normalID]->imageView(), *mNormalSampler, 1, 0);
	materialResource.bindImage(mTextures[specularID]->imageView(), *mSpecularSampler, 2, 0);

	// Generate image infos
	VkDescriptorImageInfo diffuseInfo = {};
	VkDescriptorImageInfo normalInfo = {};
	VkDescriptorImageInfo specularInfo = {};
	materialResource.generateDescriptorImageInfo(diffuseInfo, 0, 0);
	materialResource.generateDescriptorImageInfo(normalInfo, 1, 0);
	materialResource.generateDescriptorImageInfo(specularInfo, 2, 0);

	BindingMap<VkDescriptorImageInfo> imageInfos;
	imageInfos[0][0] = diffuseInfo;
	imageInfos[1][0] = normalInfo;
	imageInfos[2][0] = specularInfo;

	mPerMaterialDescriptorSets.push_back(std::make_unique<DescriptorSet>(*mDevice, *mPerMaterialDescriptorSetLayout, *mPerMaterialDescriptorPool, imageInfos));

	// Update the descriptor sets with new buffer/binding info
	std::vector<uint32_t> bindingsToUpdate = { 0, 1, 2 };
	mPerMaterialDescriptorSets.back()->update(bindingsToUpdate);

	// Return descriptor set location
	return mPerMaterialDescriptorSets.size() - 1;


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
	const aiScene* scene = importer.ReadFile(modelFile, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace);

	if (!scene)
	{
		throw std::runtime_error("Failed to load model! (" + modelFile + ")");
	}

	// Get vector of all materials with 1:1 ID placement
	std::map<uint32_t, std::string> diffuseNames;
	std::map<uint32_t, std::string> normalNames;
	std::map<uint32_t, std::string> specularNames;
	std::map<uint32_t, bool> isMaterialOpaque;
	LoadMaterials(scene, diffuseNames, normalNames, specularNames, isMaterialOpaque);

	uint32_t materialCount = scene->mNumMaterials;

	// Conversion from the materials list IDs to texture IDs
	// Note if a material has a diffuse and normal compopnent they will share the same ID
	std::vector<uint32_t> materialIDs(materialCount);

	// Loop over textureNames and create textures diffuse and normal components
	for (uint32_t i = 0; i < materialCount; ++i)
	{
		uint32_t diffuseID;
		uint32_t normalID;
		uint32_t specularID;

		// If material has no texture, set '0' to indicate no texture, texture 0 will be reserved for a default texture
		if (diffuseNames[i].empty())
		{
			diffuseID = 0;
		}
		else
		{
			// Otherwise, create texture and set value to index of new texture
			diffuseID = createTexture(diffuseNames[i]);
		}

		// repeat for normal textures
		if (normalNames[i].empty())
		{
			normalID = 0;
		}
		else
		{
			normalID = createTexture(normalNames[i]);
		}

		// repeat for specular textures
		if (specularNames[i].empty())
		{
			specularID = 0;
		}
		else
		{
			specularID = createTexture(specularNames[i]);
		}


		// Create material descriptor
		materialIDs[i] = createMaterialDescriptor(diffuseID, normalID, specularID);
	}

	// Load in all our meshes
	std::vector<std::unique_ptr<Mesh>> modelMeshes = LoadNode(*mDevice, scene->mRootNode, scene, materialIDs, isMaterialOpaque);

	mModelList.emplace_back(modelMeshes);

	return mModelList.size() - 1;
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



