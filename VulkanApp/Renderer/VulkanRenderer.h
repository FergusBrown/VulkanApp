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

class Mesh;
class MeshModel;

class Device;
class Swapchain;
class Image;
class Instance;
class Buffer;
class CommandBuffer;
class Sampler;
class Surface;
class DescriptorPool;
class DescriptorSetLayout;
class DescriptorResourceReference;
class DescriptorSet;
class Pipeline;
class PipelineLayout;
class RenderPass;
class RenderTarget;
class Subpass;
class Frame;
class Framebuffer;
class Texture;

class VulkanRenderer
{
public:
	VulkanRenderer();

	int init(GLFWwindow* newWindow);

	// Model control
	int createModel(std::string modelFile);
	void updateModel(int modelId, glm::mat4& newModel);

	// Camera Control
	void createCamera(float FoVinDegrees);
	void updateCameraView(glm::mat4& newView);


	void draw();
	void cleanup();

	~VulkanRenderer();

private:
	GLFWwindow* mWindow;

	uint32_t activeFrameIndex{ 0 };

	// Scene objects
	std::vector<MeshModel> modelList;

	// Scene Settings
	struct UboViewProjection {
		glm::mat4 projection;
		glm::mat4 view;
	} uboViewProjection;

	// Vulkan Components
	// - Main
	std::unique_ptr<Instance> mInstance;

	std::unique_ptr<Surface> mSurface{ nullptr };
	std::unique_ptr<Device> mDevice{ nullptr };
	uint32_t mGraphicsQueueFamily;

	std::unique_ptr<Swapchain> mSwapchain;
	std::vector<std::unique_ptr<Framebuffer>> mFramebuffers;
	std::vector<std::unique_ptr<Frame>> mFrames;
	
	// - Formats
	VkFormat mColourFormat{ VK_FORMAT_R8G8B8A8_UNORM };
	VkFormat mDepthFormat{ VK_FORMAT_D32_SFLOAT_S8_UINT };

	// - Descriptors
	std::unique_ptr<DescriptorSetLayout> mUniformSetLayout;
	std::unique_ptr<DescriptorSetLayout> mSamplerSetLayout;
	std::unique_ptr<DescriptorSetLayout> mAttachmentSetLayout;
	VkPushConstantRange mPushConstantRange;

	
	std::unique_ptr<DescriptorPool> mUniformDescriptorPool;
	std::unique_ptr<DescriptorPool> mSamplerDescriptorPool;
	std::unique_ptr<DescriptorPool> mAttachmentDescriptorPool;

	std::vector<std::unique_ptr<DescriptorResourceReference>> mUniformResources;
	std::vector<std::unique_ptr<DescriptorResourceReference>> mSamplerResources;
	std::vector<std::unique_ptr<DescriptorResourceReference>> mAttachmentResources;
	
	std::vector<std::unique_ptr<DescriptorSet>> mUniformDescriptorSets;			// Descriptor set holding uniform data
	std::vector<std::unique_ptr<DescriptorSet>> mTextureDescriptorSets;		// Desctcriptor sets holding texture samplers
	std::vector<std::unique_ptr<DescriptorSet>> mAttachmentDescriptorSets;		// Descriptor set holding colour/depth images (used for second subpass)

	std::vector<std::unique_ptr<Buffer>> mUniformBuffers;
	/* ABSTRACTED TO BUFFER CLASS */

	//std::vector<VkBuffer> modelDUniformBuffer;	
	//std::vector<VkDeviceMemory> modelDUniformBufferMemory;
	//VkDeviceSize minUniformBufferOffset;
	//size_t modelUniformAlignment;
	//UboModel* modelTransferSpace;

	// - Assets
	std::unique_ptr<Sampler> mTextureSampler;
	std::map <uint32_t, std::unique_ptr<Texture>> mTextures;

	// - Pipelines + Layouts
	std::vector<std::unique_ptr<Pipeline>> mPipelines;
	std::vector<std::unique_ptr<PipelineLayout>> mPipelineLayouts;


	/*VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout;
	
	VkPipeline secondPipeline;
	VkPipelineLayout secondPipelineLayout;*/

	//VkRenderPass mRenderPass;
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
	void findDesiredQueueFamilies();
	void createSwapchain();
	void createPerFrameObjects();
	void createRenderPass();
	void createDescriptorSetLayouts();
	void createPushConstantRange();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createTextureSampler();
	void createUniformBuffers();
	void createDescriptorPools();
	void createUniformDescriptorSets();
	void createInputDescriptorSets();

	// -- Support
	void updateUniformBuffers(uint32_t imageIndex);

	// - Record Functions
	void recordCommands(CommandBuffer& primaryCmdBuffer, uint32_t currentImage);
	CommandBuffer* recordSecondaryCommandBuffers(CommandBuffer* primaryCommandBuffer, uint32_t objectStart, uint32_t objectEnd, size_t threadIndex);

	// - Allocate Functions
	void allocateDynamicBufferTransferSpace();

	// - Support Functions
	
	// -- Getter Functions
	void getWindowExtent(VkExtent2D& windowExtent);
	
	// -- Choose Functions
	VkFormat chooseSupportedFormat(const std::vector<VkFormat> &formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);

	// -- Create Functions
	VkShaderModule createShaderModule(const std::vector<char>& code);

	int createTexture(std::string fileName);
	int createTextureDescriptor(const Texture& texture);
	
	// -- Loader Functions
	stbi_uc* loadTextureFile(std::string fileName, int& width, int& height, VkDeviceSize& imageSize);

	
	
};

