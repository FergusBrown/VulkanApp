#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stdexcept>
#include <vector>
#include <set>
#include <algorithm>
#include <array>

#include <iostream>
#include <cstring>

#include <future>
#include "ctpl_stl.h"

#include "Mesh.h"
#include "MeshModelData.h"
#include "MeshModel.h"

#include "Utilities.h"
#include "VulkanValidation.h"

#include "Device.h"
#include "SwapChain.h"

using namespace glm;

class VulkanRenderer
{
public:
	VulkanRenderer();

	int init(GLFWwindow* newWindow);

	// Model control
	int loadMeshModelData(std::string modelFile);
	int createModel(int modelDataIndex);
	//bool destroyModel(int modelDataIndex);
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
	std::unique_ptr<Swapchain> mSwapchain;
	//VkSwapchainKHR swapchain;

	// All 3 of below are 1:1 connected
	//std::vector<SwapchainImage> swapChainImages;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	std::vector<VkCommandBuffer> primaryCommandBuffers;

	std::vector<VkImage> colourBufferImage;
	std::vector <VkDeviceMemory> colourBufferImageMemory;
	std::vector <VkImageView> colourBufferImageView;
	VkFormat colourFormat;

	std::vector<VkImage> depthBufferImage;
	std::vector <VkDeviceMemory> depthBufferImageMemory;
	std::vector <VkImageView> depthBufferImageView;
	VkFormat depthFormat;

	// - Descriptors
	VkDescriptorSetLayout descriptorSetLayout; // describes the layout of the descriptor sets
	VkDescriptorSetLayout samplerSetLayout; 
	VkDescriptorSetLayout inputSetLayout; 
	VkPushConstantRange pushConstantRange;

	VkDescriptorPool descriptorPool;
	VkDescriptorPool samplerDescriptorPool;
	VkDescriptorPool inputDescriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;			// Descriptor set holding uniform data
	std::vector<VkDescriptorSet> samplerDescriptorSets;		// Desctcriptor sets holding texture samplers
	std::vector<VkDescriptorSet> inputDescriptorSets;		// Descriptor set holding colour/depth images (used for second subpass)

	std::vector<VkBuffer> vpUniformBuffer;		// We want one of these for every command buffer so that nothing funky happens
	std::vector<VkDeviceMemory> vpUniformBufferMemory;

	//std::vector<VkBuffer> modelDUniformBuffer;	
	//std::vector<VkDeviceMemory> modelDUniformBufferMemory;
	//VkDeviceSize minUniformBufferOffset;
	//size_t modelUniformAlignment;
	//UboModel* modelTransferSpace;

	// - Assets
	VkSampler textureSampler;
	std::vector<VkImage> textureImages;
	std::vector<VkDeviceMemory> textureImageMemory;
	std::vector<VkImageView> textureImageViews;

	// - Pipeline
	VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout;
	
	VkPipeline secondPipeline;
	VkPipelineLayout secondPipelineLayout;

	VkRenderPass renderPass;

	// - Pools
	VkCommandPool graphicsCommandPool;
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
	uint32_t numThreads;
	ctpl::thread_pool threadPool;

	//struct ThreadPushConstantBlock {
	//	Model model;
	//};

	// Attach command pool and buffer to each thread
	/* Frame */
	struct ThreadData {
		VkCommandPool commandPool;
		// One command buffer per task
		std::vector<VkCommandBuffer> commandBuffer;
	};
	std::vector<ThreadData> threadData;

	uint32_t numSecondaryBuffers;
	struct frameTEMP {
		std::vector<ThreadData> threadData;
	};

	std::vector<frameTEMP> frameData;

	// Vulkan Functions
	// - Create Functions
	void createInstance();
	void setupDebugMessenger();
	//void createLogicalDevice();
	void createSurface();
	void createDevice();
	void createSwapChain();
	void createRenderPass();
	void createDescriptorSetLayout();
	void createPushConstantRange();
	void createGraphicsPipeline();
	void createColourBufferImage();
	void createDepthBufferImage();
	void createFrameBuffers();
	void createThreadPool();
	void createCommandPools();
	void createCommandBuffers();
	void createSynchronation();
	void createTextureSampler();
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();
	void createInputDescriptorSets();

	void updateUniformBuffers(uint32_t imageIndex);

	// - Record Functions
	void recordCommands(uint32_t currentImage);
	VkCommandBuffer* VulkanRenderer::recordSecondaryCommandBuffers(VkCommandBufferBeginInfo beginInfo, uint32_t objectStart, uint32_t objectEnd, uint32_t currentImage, uint32_t taskIndex, size_t threadID);
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
	VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, 
		VkMemoryPropertyFlags propFlags, VkDeviceMemory *imageMemory);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	VkShaderModule createShaderModule(const std::vector<char>& code);

	int createTextureImage(std::string fileName);
	int createTexture(std::string fileName);
	int createTextureDescriptor(VkImageView textureImage);

	

	// -- Loader Functions
	stbi_uc* loadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize);
	
};

