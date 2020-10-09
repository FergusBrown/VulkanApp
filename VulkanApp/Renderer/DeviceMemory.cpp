#include "DeviceMemory.h"

#include "Device.h"

DeviceMemory::DeviceMemory(Device& device, VkMemoryPropertyFlags properties, VkMemoryRequirements& memRequirements) :
	mDevice(device)
{

	// ALLOCATE MEMORY TO BUFFER
	VkMemoryAllocateInfo memoryAllocInfo = {};
	memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocInfo.allocationSize = memRequirements.size;
	memoryAllocInfo.memoryTypeIndex =
		findMemoryTypeIndex(mDevice.physicalDevice(), memRequirements.memoryTypeBits,					// Index of memory type on Physical device that has required bit flags
			properties);																				// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT	: CPU can interact with memory
																										// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT	: Allows placement of data straight into buffer after mapping (otherwise would have to specify manually)

	// Allocate memory to VkDeviceMemory
	VkResult result = vkAllocateMemory(device.logicalDevice(), &memoryAllocInfo, nullptr, &mHandle);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate Vertex Buffer memory!");
	}
}

DeviceMemory::~DeviceMemory()
{
    if (mHandle != VK_NULL_HANDLE)
    {
        vkFreeMemory(mDevice.logicalDevice(), mHandle, nullptr);
    }
 
}

Device& DeviceMemory::device() const
{
    return mDevice;
}

VkDeviceMemory DeviceMemory::handle() const
{
    return mHandle;
}

uint32_t DeviceMemory::findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t allowedTypes, VkMemoryPropertyFlags properties)
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

	throw std::runtime_error("The physical device does not possess a memory type with the required properties!");
}
