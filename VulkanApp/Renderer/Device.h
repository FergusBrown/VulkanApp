#pragma once

#include "Common.h"

class CommandBuffer;
class CommandPool;
class Instance;
class PhysicalDevice;
class Queue;

// Container for logical and physical device
class Device
{
public:
	// Create logical device based on list of requested extensions
	Device(Instance& instance, VkSurfaceKHR surface, const std::vector<const char*>& requiredExtensions, VkPhysicalDeviceFeatures& requiredFeatures);
	~Device();

	// - Getters
	PhysicalDevice& physicalDevice() const;
	VkDevice logicalDevice() const;
	CommandPool& primaryCommandPool();
	const Queue& queue(uint32_t familyIndex, uint32_t index) const;
	const VkPhysicalDeviceProperties& physicalDeviceProperties();

	const Queue& getQueueByFlag(VkQueueFlagBits queueFlag, uint32_t index);
	uint32_t getQueueFamilyIndex(VkQueueFlagBits queueFlag);

	CommandBuffer& requestCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	// - Management
	std::unique_ptr<CommandBuffer> createAndBeginTemporaryCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	void endAndSubmitTemporaryCommandBuffer(CommandBuffer& commandBuffer);
	VkResult waitIdle() const;
	//void submitCommandBuffer(CommandBuffer commandBuffer);

private:
	// Variables
	std::unique_ptr<PhysicalDevice> mPhysicalDevice;
	VkDevice mLogicalDevice;
	VkSurfaceKHR mSurface;

	std::vector<std::vector<Queue>> mQueues;

	// Command pool associated with the primary queue
	std::unique_ptr<CommandPool> mPrimaryCommandPool;

	// Functions
	// - Get Physical Device referece
	void getPhysicalDevice(VkInstance instance, const std::vector<const char*>& requiredExtensions, VkPhysicalDeviceFeatures& requiredFeatures);
	
	// - Object creation
	void createLogicalDevice(const std::vector<const char*>& requiredExtensions, VkPhysicalDeviceFeatures& requiredFeatures);
	void createCommandPool();
	
};
