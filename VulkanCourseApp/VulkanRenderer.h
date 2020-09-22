#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <ctpl_stl.h>

#include "Common.h"
#include "Utilities.h"
#include "VulkanValidation.h"

class Mesh;
class MeshModelData;
class MeshModel;

class Device;
class Swapchain;
class Image;
class Buffer;
class CommandBuffer;
class Sampler;
class DescriptorPool;
class DescriptorSetLayout;
class DescriptorResourceReference;
class DescriptorSet;
class RenderTarget;
class Frame;
class Framebuffer;
class Texture;

using namespace glm;

class VulkanRenderer
{
public:
	VulkanRenderer();

	int init(GLFWwindow* newWindow);

	// Model control
	int loadMeshModelData(std::string modelFile);
	int createModel(int modelDataIndex);
	void updateModel(int modelId, mat4 newModel);

	// Camera Control
	void createCamera(float FoVinDegrees);
	void updateCameraView(mat4 newView);


	void draw();
	void cleanup();

	~VulkanRenderer();

private:
	GLFWwindow* window;

	int currentFrame = 0;

	// Scene objects
	std::vector<MeshModelData> modelDataList;			// Model data

	std::vector<MeshModel> modelList;	// Holds pointers to all MeshModels in order of creation time

	// Scene Settings
	struct UboViewProjection {
		glm::mat4 projection;
		glm::mat4 view;
	} uboViewProjection;

	// Vulkan Components
	// - Main
	VkInstance mInstance;
	VkDebugUtilsMessengerEXT debugMessenger;
	/*struct
	{
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
	} mainDevice;
	VkQueue graphicsQueue;			// abstract to Device
	VkQueue presentationQueue;		// abstract to Device*/
	VkSurfaceKHR mSurface;
	std::unique_ptr<Device> mDevice;
	uint32_t mGraphicsQueueFamily;

	std::unique_ptr<Swapchain> mSwapchain;
	//std::vector<std::unique_ptr<RenderTarget>> mRenderTargets; ----> RENDER TARGETS ARE OWNED BY THE FRAME OBJECTS
	std::vector<std::unique_ptr<Framebuffer>> mFramebuffers;
	std::vector<std::unique_ptr<Frame>> mFrames;
	

	//VkSwapchainKHR swapchain;

	// All 3 of below are 1:1 connected
	//std::vector<SwapchainImage> swapChainImages;
	//std::vector<VkFramebuffer> swapChainFramebuffers;
	//std::vector<VkCommandBuffer> primaryCommandBuffers;

	/*std::vector<VkImage> colourBufferImage;
	std::vector <VkDeviceMemory> colourBufferImageMemory;
	std::vector <VkImageView> colourBufferImageView;*/
	VkFormat mColourFormat{ VK_FORMAT_R8G8B8A8_UNORM };

	/*std::vector<VkImage> depthBufferImage;
	std::vector <VkDeviceMemory> depthBufferImageMemory;
	std::vector <VkImageView> depthBufferImageView;*/
	VkFormat mDepthFormat{ VK_FORMAT_D32_SFLOAT_S8_UINT };

	// - Descriptors
	//VkDescriptorSetLayout descriptorSetLayout; // describes the layout of the descriptor sets
	//VkDescriptorSetLayout samplerSetLayout; 
	//VkDescriptorSetLayout inputSetLayout; 
	std::unique_ptr<DescriptorSetLayout> mUniformSetLayout;
	std::unique_ptr<DescriptorSetLayout> mSamplerSetLayout;
	std::unique_ptr<DescriptorSetLayout> mAttachmentSetLayout;
	VkPushConstantRange pushConstantRange;

	/*VkDescriptorPool descriptorPool;
	VkDescriptorPool samplerDescriptorPool;
	VkDescriptorPool inputDescriptorPool;*/
	
	std::unique_ptr<DescriptorPool> mUniformDescriptorPool;
	std::unique_ptr<DescriptorPool> mSamplerDescriptorPool;
	std::unique_ptr<DescriptorPool> mAttachmentDescriptorPool;

	//std::vector<VkDescriptorSet> descriptorSets;			// Descriptor set holding uniform data
	//std::vector<VkDescriptorSet> samplerDescriptorSets;		// Desctcriptor sets holding texture samplers
	//std::vector<VkDescriptorSet> inputDescriptorSets;		// Descriptor set holding colour/depth images (used for second subpass)
	
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
	//VkSampler textureSampler;
	std::unique_ptr<Sampler> mTextureSampler;
	//std::vector<VkImage> textureImages;
	//std::vector<VkDeviceMemory> textureImageMemory;
	//std::vector<VkImageView> textureImageViews;
	std::map <uint32_t, std::unique_ptr<Texture>> mTextures;

	// - Pipeline
	VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout;
	
	VkPipeline secondPipeline;
	VkPipelineLayout secondPipelineLayout;

	VkRenderPass mRenderPass;

	// - Pools
	//VkCommandPool graphicsCommandPool;
	//std::vector<VkCommandPool> secondaryCommandPools;

	// - Utility
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	// - Synchronisation /* In Frame Class
	std::vector<VkSemaphore> imageAvailable;
	std::vector<VkSemaphore> renderFinished;
	std::vector<VkFence> drawFences;

	// - Multithreading
	// Max. number of concurrent threads
	uint32_t mThreadCount;
	ctpl::thread_pool mThreadPool;

	//struct ThreadPushConstantBlock {
	//	Model model;
	//};

	/* Below abstracted to Frame*/
	// Attach command pool and buffer to each thread
	/* Frame */
	//struct ThreadData {
	//	VkCommandPool commandPool;
	//	// One command buffer per task
	//	std::vector<VkCommandBuffer> handle;
	//};
	//std::vector<ThreadData> threadData;

	//uint32_t numSecondaryBuffers;
	//struct frameTEMP {
	//	std::vector<ThreadData> threadData;
	//};

	//std::vector<frameTEMP> frameData;

	// Vulkan Functions
	// - Create Functions
	void setupThreadPool();
	void createInstance();
	void setupDebugMessenger();
	//void createLogicalDevice();
	void createSurface();
	void createDevice();
	void findDesiredQueueFamilies();
	void createSwapChain();
	void createPerFrameObjects();
	void createRenderPass();
	void createDescriptorSetLayouts();
	void createPushConstantRange();
	void createGraphicsPipeline();
	//void createColourBufferImage();
	//void createDepthBufferImage();
	//void createFrameBuffers();
	void createFramebuffers();
	/*void createThreadData();*/
	/*void createCommandPools();
	void createCommandBuffers();*/
	void createSynchronation();
	void createTextureSampler();
	void createUniformBuffers();
	void createDescriptorPools();
	//void createDescriptorResourceReferences();
	void createUniformDescriptorSets();
	void createInputDescriptorSets();

	// -- Support
	//void createFramebuffer();
	void updateUniformBuffers(uint32_t imageIndex);

	// - Record Functions
	void recordCommands(CommandBuffer& primaryCmdBuffer, uint32_t currentImage);
	CommandBuffer* recordSecondaryCommandBuffers(Frame& frame, uint32_t objectStart, uint32_t objectEnd, size_t threadIndex, uint32_t currentImage);
	// -- Create Helper Functions
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	// - Get Functions
	//void getPhysicalDevice();

	// - Allocate Functions
	void allocateDynamicBufferTransferSpace();

	// - Support Functions
	// -- Checker Functions
	bool checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions);
	bool checkValidationLayerSupport();
	//bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	//bool checkDeviceSuitable(VkPhysicalDevice device);
	
	// -- Getter Functions
	//QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);
	//SwapChainDetails getSwapChainDetails(VkPhysicalDevice device);
	
	// -- Choose Functions
	//VkSurfaceFormatKHR chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);			// Swapchain
	//VkPresentModeKHR chooseBestPresentationMode(const std::vector<VkPresentModeKHR> presentationMode);	// Swapchain
	//VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);					// Swapchain
	VkFormat chooseSupportedFormat(const std::vector<VkFormat> &formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);

	// -- Create Functions
	//VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, 
	//	VkMemoryPropertyFlags propFlags, VkDeviceMemory *imageMemory);
	//VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	VkShaderModule createShaderModule(const std::vector<char>& code);

	//int createTextureImage(std::string fileName);
	int createTexture(std::string fileName);
	int createTextureDescriptor(const Texture& texture);
	

	

	// -- Loader Functions
	//stbi_uc* loadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize);
	
};

