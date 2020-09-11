#pragma once
#include "Common.h"

#include "Device.h"
#include "CommandBuffer.h"

class CommandPool
{
public:
	// TODO: probably don't need this index
	CommandPool(Device& device, uint32_t queueFamilyIndex);
	~CommandPool();

	// - Getters
	Device& device();
	uint32_t queueFamilyIndex() const;
	VkCommandPool handle() const;

	// - Pool Operations
	VkResult reset();
	CommandBuffer& requestCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);


private:
	Device& mDevice;

	uint32_t mQueueFamilyIndex;

	VkCommandPool mHandle{ VK_NULL_HANDLE };

	std::vector<std::unique_ptr<CommandBuffer>> mPrimaryCommandBuffers;

	uint32_t mActivePrimaryCommandBuffers{ 0 };

	std::vector<std::unique_ptr<CommandBuffer>> mSecondaryCommandBuffers;

	uint32_t mActiveSecondaryCommandBuffers{ 0 };
};

