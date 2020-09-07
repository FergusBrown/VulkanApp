#pragma once
#include "Common.h"

#include "Device.h"

// Contains a VkBuffer and the VkDeviceMemory which is mapped to the resource
class Buffer
{
public:
	Buffer(Device& device,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties);
	~Buffer();

	// - Getters
	VkBuffer handle() const;
	VkDeviceMemory memory() const;

	// Maps and returns a pointer to the buffer memory (allowing for operations on the memory e.g. memcpy)
	void* map();

	// Unmaps the memory
	void unmap();

private:
	Device& mDevice;

	VkBuffer mHandle{ VK_NULL_HANDLE };

	VkDeviceMemory mMemory;

	VkDeviceSize mSize{ 0 };

	// - Support
	static uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t allowedTypes, VkMemoryPropertyFlags properties);
};

