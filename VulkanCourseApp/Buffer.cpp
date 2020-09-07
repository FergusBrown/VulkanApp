#include "Buffer.h"

Buffer::Buffer(Device& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) :
    mDevice(device), mSize(size)
{
	// CREATE BUFFER
	// Information to create a buffer (doesn't include assigning memory)
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;										// Size of buffer
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;			

	VkResult result = vkCreateBuffer(device.logicalDevice(), &bufferInfo, nullptr, &mHandle);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Buffer!");
	}

	// GET BUFFER MEMORY REQUIREMENTS
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device.logicalDevice(), mHandle, &memRequirements);

	// ALLOCATE MEMORY TO BUFFER
	VkMemoryAllocateInfo memoryAllocInfo = {};
	memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocInfo.allocationSize = memRequirements.size;
	memoryAllocInfo.memoryTypeIndex = 
		Buffer::findMemoryTypeIndex(mDevice.physicalDevice(), memRequirements.memoryTypeBits,		// Index of memory type on Physical device that has required bit flags
		properties);																				// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT	: CPU can interact with memory
																									// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT	: Allows placement of data straight into buffer after mapping (otherwise would have to specify manually)

	// Allocate memory to VkDeviceMemory
	result = vkAllocateMemory(device.logicalDevice(), &memoryAllocInfo, nullptr, &mMemory);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate Vertex Buffer memory!");
	}

	// Allocate memory to given buffer
	vkBindBufferMemory(device.logicalDevice(), mHandle, mMemory, 0);
}

Buffer::~Buffer()
{
	vkDestroyBuffer(mDevice.logicalDevice(), mHandle, nullptr);
	vkFreeMemory(mDevice.logicalDevice(), mMemory, nullptr);
}

VkBuffer Buffer::handle() const
{
    return mHandle;
}

VkDeviceMemory Buffer::memory() const
{
    return mMemory;
}

void* Buffer::map()
{
	void* data;
	vkMapMemory(mDevice.logicalDevice(), mMemory, 0, mSize, 0, &data);
	return data;
}

void Buffer::unmap()
{
	vkUnmapMemory(mDevice.logicalDevice(), mMemory);
}

uint32_t Buffer::findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t allowedTypes, VkMemoryPropertyFlags properties)
{
	// Get properties of physical device memory
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if ((allowedTypes & (1 << i))														// Index of memory type must match corresponding bit in allowedTypes
			&& (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)	// Desired property bit flags are part of memory type's property flags
		{
			// This memory type is valid so return its index
			return i;
		}
	}
}
