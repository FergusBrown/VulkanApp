#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <set>
#include <algorithm>
#include <array>

#include <iostream>
#include <cstring>

#include "Utilities.h"
#include "VulkanValidation.h"


class VulkanRenderer
{
public:
	VulkanRenderer();

	int init(GLFWwindow* newWindow);
	void draw();
	void cleanup();

	~VulkanRenderer();

private:
	GLFWwindow* window;

	int currentFrame = 0;

	// Vulkan Components
	// - Main
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	struct
	{
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
	} mainDevice;
	VkQueue graphicsQueue;
	VkQueue presentationQueue;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;

	// All 3 of below are 1:1 connected
	std::vector<SwapchainImage> swapChainImages;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	std::vector<VkCommandBuffer> commandBuffers;

	// - Pipeline
	VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;

	// - Pools
	VkCommandPool graphicsCommandPool;

	// - Utility
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	// - Synchronisation
	std::vector<VkSemaphore> imageAvailable;
	std::vector<VkSemaphore> renderFinished;
	std::vector<VkFence> drawFences;

	// Vulkan Functions
	// - Create Functions
	void createInstance();
	void setupDebugMessenger();
	void createLogicalDevice();
	void createSurface();
	void createSwapChain();
	void createRenderPass();
	void createGraphicsPipeline();
	void createFrameBuffers();
	void createCommandPool();
	void createCommandBuffers();
	void createSynchronation();

	// - Record Functions
	void recordCommands();

	// -- Create Helper Functions
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	// - Get Functions
	void getPhysicalDevice();

	// - Support Functions
	// -- Checker Functions
	bool checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions);
	bool checkValidationLayerSupport();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	bool checkDeviceSuitable(VkPhysicalDevice device);
	
	// -- Getter Functions
	QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);
	SwapChainDetails getSwapChainDetails(VkPhysicalDevice device);
	
	// -- Choose Functions
	VkSurfaceFormatKHR chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR chooseBestPresentationMode(const std::vector<VkPresentModeKHR> presentationMode);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

	// -- Create Functions
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	
};

