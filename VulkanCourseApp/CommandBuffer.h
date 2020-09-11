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
	const VkCommandBuffer& handle() const;
	const VkCommandBufferLevel level() const;

	// Command buffer operations
	void begin(VkCommandBufferUsageFlags flags);

	void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

	void end();
	void submit(VkQueue queue);

private:
	CommandPool& mCommandPool;

	VkCommandBuffer mHandle{ VK_NULL_HANDLE };

	const VkCommandBufferLevel mLevel;

};

