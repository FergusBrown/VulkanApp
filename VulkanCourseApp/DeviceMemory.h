#pragma once
#include "Common.h"

#include "Device.h"

class DeviceMemory
{
public:
	DeviceMemory(Device& device, VkMemoryPropertyFlags properties, VkMemoryRequirements& memRequirements);
	~DeviceMemory();

	// - Getters
	Device& device() const;
	VkDeviceMemory handle() const;
private:
	Device& mDevice;

	VkDeviceMemory mHandle{ VK_NULL_HANDLE };

	//VkMemoryPropertyFlags	mPropFlags;

	// - Support
	static uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t allowedTypes, VkMemoryPropertyFlags properties);
};

