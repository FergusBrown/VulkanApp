#include "CommandBuffer.h"

CommandBuffer::CommandBuffer(CommandPool& commandPool, VkCommandBufferLevel level) :
	mCommandPool(commandPool), mLevel(level)
{

	VkCommandBufferAllocateInfo cbAllocInfo = {};
	cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbAllocInfo.commandPool = commandPool.commandPool();
	cbAllocInfo.level = level;						// VK_COMMAND_BUFFER_LEVEL_PRIMARY   : buffer you submit directly to the queue. Cannot be called by other buffers.
													// VK_COMMAND_BUFFER_LEVEL_SECONDARY : buffer cannot be called directly. Can be called by other buffers via "vkCmdExecuteCommands" when recording commands in primary buffer
	cbAllocInfo.commandBufferCount = 1;

	// Allocate command buffers and place handles in array of buffers
	VkResult result = vkAllocateCommandBuffers(commandPool.device().logicalDevice(), &cbAllocInfo, &mCommandBuffer);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate Command Buffer(s)!");
	}
}

CommandBuffer::~CommandBuffer()
{
	// Free command buffer when out of scope
	if (mCommandBuffer != VK_NULL_HANDLE)
	{
		vkFreeCommandBuffers(mCommandPool.device().logicalDevice(), mCommandPool.commandPool(), 1, nullptr);
	}
}

const VkCommandBuffer& CommandBuffer::commandBuffer() const
{
	return mCommandBuffer;
}

const VkCommandBufferLevel CommandBuffer::level() const
{
	return mLevel;
}
