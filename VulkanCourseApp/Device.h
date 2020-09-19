#pragma once

#include "Common.h"

#include "CommandPool.h"
#include "Queue.h"

//	// Indicies of Queue Families (if they exist at all)
//struct QueueFamilyIndices {
//	int graphicsFamily = -1;
//	int presentationFamily = -1;
//
//	// Check if queue failies are valid
//	bool isValid()
//	{
//		return graphicsFamily >= 0 && presentationFamily >= 0;
//	}
//};

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
	CommandPool& primaryCommandPool();
	const Queue& queue(uint32_t familyIndex, uint32_t index) const;
	const Queue& getQueueByFlag(VkQueueFlagBits queueFlag, uint32_t index);

	CommandBuffer& requestCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	// - Management
	std::unique_ptr<CommandBuffer> createAndBeginTemporaryCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	void endAndSubmitTemporaryCommandBuffer(CommandBuffer& commandBuffer);
	//void submitCommandBuffer(CommandBuffer commandBuffer);

private:
	// Variables
	VkPhysicalDevice mPhysicalDevice;
	VkDevice mLogicalDevice;
	VkSurfaceKHR mSurface;

	//QueueFamilyIndices mQueueFamilyIndices;
	//VkQueue mGraphicsQueue;
	//VkQueue mPresentationQueue;

	std::vector<std::vector<Queue>> mQueues;

	// Command pool associated with the primary queue
	std::unique_ptr<CommandPool> mPrimaryCommandPool;

	// Functions
	// - Get Physical Device referece
	void getPhysicalDevice(VkInstance instance, const std::vector<const char*>& deviceExtensions);
	// -- Support
	/*QueueFamilyIndices getQueueFamilies();*/
	uint32_t getQueueFamilyIndex(VkQueueFlagBits queueFlag);
	VkBool32 checkPresentationSupport(uint32_t queueFamilyIndex);
	bool checkDeviceSuitable(const std::vector<const char*>& deviceExtensions);
	bool checkDeviceExtensionSupport(const std::vector<const char*>& deviceExtensions);

	// - Object creation
	void createLogicalDevice(const std::vector<const char*>& deviceExtensions);
	void createCommandPool();
	
};
