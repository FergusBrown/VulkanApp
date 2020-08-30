#pragma once
#include "Common.h"

#include "CommandPool.h"

// Class handles all command buffer operations
class CommandBuffer
{
public:
	CommandBuffer(CommandPool& commandPool, VkCommandBufferLevel level);
	~CommandBuffer();

	// - Getters
	const VkCommandBuffer& commandBuffer() const;
	const VkCommandBufferLevel level() const;

private:
	CommandPool& mCommandPool;

	VkCommandBuffer mCommandBuffer{ VK_NULL_HANDLE };

	const VkCommandBufferLevel mLevel;
};

