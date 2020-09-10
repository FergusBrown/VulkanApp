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

	mMemory = std::make_unique<DeviceMemory>(device, properties, memRequirements);

	// Allocate memory to given buffer
	vkBindBufferMemory(device.logicalDevice(), mHandle, mMemory->handle(), 0);
}

Buffer::~Buffer()
{
	if (mHandle != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(mDevice.logicalDevice(), mHandle, nullptr);
	}
}

VkBuffer Buffer::handle() const
{
    return mHandle;
}

VkDeviceMemory Buffer::memory() const
{
    return mMemory->handle();
}

void* Buffer::map()
{
	void* data;
	vkMapMemory(mDevice.logicalDevice(), mMemory->handle(), 0, mSize, 0, &data);
	return data;
}

void Buffer::unmap()
{
	vkUnmapMemory(mDevice.logicalDevice(), mMemory->handle());
}
