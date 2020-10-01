#include "CommandPool.h"

#include "CommandBuffer.h"
#include "Device.h"

CommandPool::CommandPool(Device& device, uint32_t queueFamilyIndex) :
	mDevice(device), mQueueFamilyIndex(queueFamilyIndex)
{
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;			// Transient bit as reset pool will be used for recycling buffers
	poolInfo.queueFamilyIndex = queueFamilyIndex;					// Queue family type that buffers from this command pool will use

	// Create a graphics queue family command pool
	VkResult result = vkCreateCommandPool(mDevice.logicalDevice(), &poolInfo, nullptr, &mHandle);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Command Pool!");
	}
}

CommandPool::~CommandPool()
{
	mPrimaryCommandBuffers.clear();
	mSecondaryCommandBuffers.clear();

	if (mHandle != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool(mDevice.logicalDevice(), mHandle, nullptr);
	}
}

Device& CommandPool::device()
{
	return mDevice;
}


uint32_t CommandPool::queueFamilyIndex() const
{
	return mQueueFamilyIndex;
}

VkCommandPool CommandPool::handle() const
{
	return mHandle;
}

VkResult CommandPool::reset()
{
	// This resets all command buffers, allowing them to be recycled without reallocation
	VkResult result = vkResetCommandPool(mDevice.logicalDevice(), mHandle, 0);

	if (result == VK_SUCCESS)
	{
		mActivePrimaryCommandBuffers = 0;
		mActiveSecondaryCommandBuffers = 0;
	}
	return result;
}

CommandBuffer& CommandPool::requestCommandBuffer(VkCommandBufferLevel level)
{
	if (level == VK_COMMAND_BUFFER_LEVEL_PRIMARY)
	{
		// TODO : with current functionality this check does nothing as active count should always be the same as number of buffers
		// However, if we added the ability to reset buffers this would become useful
		if (mActivePrimaryCommandBuffers < mPrimaryCommandBuffers.size())
		{
			return *mPrimaryCommandBuffers[mActivePrimaryCommandBuffers++];
		}

		mPrimaryCommandBuffers.push_back(std::make_unique<CommandBuffer>(*this, level));
		++mActivePrimaryCommandBuffers;
		return *mPrimaryCommandBuffers.back();
	}
	else 
	{
		if (mActiveSecondaryCommandBuffers < mSecondaryCommandBuffers.size())
		{
			return *mSecondaryCommandBuffers[mActiveSecondaryCommandBuffers++];
		}

		mSecondaryCommandBuffers.push_back(std::make_unique<CommandBuffer>(*this, level));
		++mActiveSecondaryCommandBuffers;
		return *mSecondaryCommandBuffers.back();
	}

}
