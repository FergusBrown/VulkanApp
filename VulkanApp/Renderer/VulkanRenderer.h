#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ctpl_stl.h>

#include <stb_image.h>

#include "Common.h"
#include "Utilities.h"
#include "ModelLoader.h"

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
#include "PhysicalDevice.h"
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

// Abstract class to derive vulkan applications from
// Functionality for model and texture loading and the associated descriptor sets, buffers etc. are implemented here
// A uniform buffer exists to hold the view projection matrix
class VulkanRenderer
{
public:
	VulkanRenderer() = default;
	virtual ~VulkanRenderer();

	virtual int init(GLFWwindow* newWindow);

	// Model control
	int createModel(std::string modelFile);
	void updateModel(int modelId, glm::mat4& newModel);

	// Camera Control
	void createCamera(float FoVinDegrees);
	void updateCameraView(glm::mat4& newView);

	virtual void draw() = 0;

protected:
	GLFWwindow* mWindow;

	uint32_t activeFrameIndex{ 0 };

	// List of models made up of a series of meshes
	std::vector<MeshModel> mModelList;

	// Scene Settings
	struct ViewProjection {
		glm::mat4 projection;
		glm::mat4 view;
	} mUBOViewProjection;

	// Vulkan Components
	// - Main
	std::unique_ptr<Instance> mInstance{ nullptr };

	std::unique_ptr<Surface> mSurface{ nullptr };
	std::unique_ptr<Device> mDevice{ nullptr };
	uint32_t mGraphicsQueueFamily{ 0 };

	std::unique_ptr<Swapchain> mSwapchain{ nullptr };
	std::vector<std::unique_ptr<Framebuffer>> mFramebuffers;
	std::vector<std::unique_ptr<Frame>> mFrames;
	
	// - Formats
	VkFormat mColourFormat{ VK_FORMAT_R8G8B8A8_UNORM };
	VkFormat mDepthFormat{ VK_FORMAT_D32_SFLOAT_S8_UINT };

	// - Descriptors
	// -- Layouts
	std::unique_ptr<DescriptorSetLayout> mPerMaterialDescriptorSetLayout;
	VkPushConstantRange mPushConstantRange;		// push constant acts as per draw descriptor set 

	// -- Pool
	std::unique_ptr<DescriptorPool> mPerMaterialDescriptorPool;

	// -- Sets
	std::vector<std::unique_ptr<DescriptorSet>> mPerMaterialDescriptorSets;		// Descriptor sets holding texture samplers

	// - Assets
	std::unique_ptr<Sampler> mTextureSampler;
	std::map <uint32_t, std::unique_ptr<Texture>> mTextures;

	// - Pipelines + Layouts
	std::vector<std::unique_ptr<Pipeline>>	mPipelines;
	std::vector<std::unique_ptr<PipelineLayout>> mPipelineLayouts;

	// - Renderpass
	std::vector<std::unique_ptr<Subpass>> mSubpasses;
	std::unique_ptr<RenderPass> mRenderPass;

	// - Multithreading
	// Max. number of concurrent threads
	uint32_t mThreadCount;
	ctpl::thread_pool mThreadPool;

	// Vulkan Functions
	// - Create Functions
	void setupThreadPool();

	void createInstance();
	void createSurface();
	void createDevice();
	virtual void findDesiredQueueFamilies();
	virtual void createSwapchain();

	void chooseImageFormats();
	virtual void createRenderTargetAndFrames()	= 0;	// Resource references to attachments should be created here
	virtual void createRenderPass()				= 0;

	// CREATE DESCRIPTOR SET LAYOUTS
	virtual void createPerFrameDescriptorSetLayouts()	= 0;
	virtual void createPerMaterialDescriptorSetLayout();
	virtual void createPushConstantRange();

	virtual void createPipelines()				= 0;
	void createFramebuffers();

	// CREATE DESCRIPTOR RESOURCES
	virtual void createPerFrameResources()	= 0;
	void createTextureSampler();
	
	// CREATE DESCRIPTOR POOLS	
	void createPerMaterialDescriptorPool();

	// CREATE DESCRIPTOR SETS
	virtual void createPerFrameDescriptorSets()		= 0;

	// -- Support
	virtual void updatePerFrameResources()			= 0;
	virtual void getRequiredExtenstionAndFeatures(std::vector<const char*>& requiredExtensions,
		VkPhysicalDeviceFeatures& requiredFeatures) = 0;

	// - Support Functions
	// -- Getter Functions
	void getWindowExtent(VkExtent2D& windowExtent);
	
	// -- Choose Functions
	VkFormat chooseSupportedFormat(const std::vector<VkFormat> &formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);

	int createTexture(std::string fileName);
	int createTextureDescriptor(const Texture& texture);
	
	// -- Loader Functions
	stbi_uc* loadTextureFile(std::string fileName, int& width, int& height, VkDeviceSize& imageSize);
	
};

