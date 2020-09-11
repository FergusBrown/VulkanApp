#pragma once

#include "Common.h"

#include "CommandPool.h"

	// Indicies of Queue Families (if they exist at all)
struct QueueFamilyIndices {
	int graphicsFamily = -1;
	int presentationFamily = -1;

	// Check if queue failies are valid
	bool isValid()
	{
		return graphicsFamily >= 0 && presentationFamily >= 0;
	}
};

// Container for logical and physical device
class Device
{
public:
	// Create logical device based on list of extensions
	Device(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
	~Device();

	// - Getters
	VkPhysicalDevice physicalDevice() const;
	VkDevice logicalDevice() const;
	QueueFamilyIndices queueFamilyIndices() const;
	VkQueue graphicsQueue() const;
	VkQueue presentationQueue() const;
	CommandPool& primaryCommandPool();

	// - Management
	VkCommandBuffer createAndBeginTemporaryCommandBuffer(VkCommandBufferLevel level);
	void endAndSubmitTemporaryCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue);

private:
	// Variables
	VkPhysicalDevice mPhysicalDevice;
	VkDevice mLogicalDevice;
	VkSurfaceKHR mSurface;
	QueueFamilyIndices mQueueFamilyIndices;
	VkQueue mGraphicsQueue;
	VkQueue mPresentationQueue;

	// Command pool associated with the primary queue
	std::unique_ptr<CommandPool> mPrimaryCommandPool;

	// Functions
	// - Get Physical Device referece
	void getPhysicalDevice(VkInstance instance, const std::vector<const char*>& deviceExtensions);
	// -- Support
	QueueFamilyIndices getQueueFamilies();
	bool checkDeviceSuitable(const std::vector<const char*>& deviceExtensions);
	bool checkDeviceExtensionSupport(const std::vector<const char*>& deviceExtensions);

	// - Object creation
	void createLogicalDevice(const std::vector<const char*>& deviceExtensions);
	void createCommandPool();
	
};

