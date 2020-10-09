#include "Frame.h"

#include "CommandBuffer.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "DescriptorSetLayout.h"
#include "Device.h"
#include "Queue.h"
#include "RenderTarget.h"

Frame::Frame(Device& device, std::unique_ptr<RenderTarget>&& renderTarget, size_t threadCount) :
	mDevice(device), mRenderTarget(std::move(renderTarget)),
	mFencePool(device), mSemaphorePool(device),
	mThreadCount(threadCount)
{

}

Device& Frame::device() const
{
	return mDevice;
}

const RenderTarget& Frame::renderTarget() const
{
	return *mRenderTarget;
}


void Frame::reset()
{
	// Reset synchronisation pools
	mFencePool.reset();
	mSemaphorePool.reset();

	// Reset thread data
	for (size_t i = 0; i < mThreadCount; ++i)
	{
		auto& thread = mThreadData[i];
		for (auto& pool : thread.commandPools)
		{
			pool->reset();
		}
	}
}

CommandBuffer& Frame::requestCommandBuffer(const Queue& queue, VkCommandBufferLevel level, size_t threadIndex)
{
	auto& commandPool = requestCommandPool(queue, threadIndex);

	return commandPool->requestCommandBuffer(level);
}

//DescriptorSet& Frame::requestDescriptorSet(DescriptorSetLayout& descriptorSetLayout, const BindingMap<VkDescriptorImageInfo>& imageInfos, const BindingMap<VkDescriptorBufferInfo>& bufferInfos, size_t threadIndex)
//{
//	// TODO: insert return statement here
//}

VkFence Frame::requestFence()
{
	return mFencePool.requestFence();
}

VkSemaphore Frame::requestSemaphore()
{
	return mSemaphorePool.requestSemaphore();
}

void Frame::wait()
{
	VkResult result = mFencePool.wait();
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to wait for Fence(s) to signal!");
	}
}


// Check if a pool for the requested queue exists
// If it does exist then return it
// Otherwise create the rquested pool
std::unique_ptr<CommandPool>& Frame::requestCommandPool(const Queue& queue, size_t threadIndex)
{
	auto& commandPools = mThreadData[threadIndex].commandPools;
	
	// Return pool if it exists
	for (auto& pool : commandPools)
	{
		if (pool->queueFamilyIndex() == queue.familyIndex())
		{
			return pool;
		}
	}

	// Create and return requested pool
	commandPools.push_back(std::make_unique<CommandPool>(mDevice, queue.familyIndex()));

	return commandPools.back();
}
