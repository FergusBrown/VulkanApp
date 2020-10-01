#include "Frame.h"

#include "CommandBuffer.h"
//#include "CommandPool.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "Device.h"
//#include "FencePool.h"
#include "Queue.h"
#include "RenderTarget.h"
//#include "SemaphorePool.h"

Frame::Frame(Device& device, std::unique_ptr<RenderTarget>&& renderTarget, uint32_t threadCount) :
	mDevice(device), mRenderTarget(std::move(renderTarget)),
	mFencePool(device), mSemaphorePool(device)
{
	//mThreadPool.resize(mThreadCount);
	mThreadData.resize(threadCount);

	// THREAD SPECIFIC DATA : COMMAND + DESCRIPTOR POOLS
	//createThreadData();

}

//Frame::~Frame()
//{
//
//}

//Frame::~Frame()
//{
//	reset();
//}

Device& Frame::device() const
{
	return mDevice;
}

const RenderTarget& Frame::renderTarget() const
{
	return *mRenderTarget;
}

//CommandPool& Frame::commandPool(uint32_t threadIndex)
//{
//	return *mThreadData[threadIndex].commandPools;
//}

void Frame::reset()
{
	// Reset synchronisation pools
	mFencePool.reset();
	mSemaphorePool.reset();

	// Reset thread data
	for (auto& thread : mThreadData)
	{
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

//void Frame::createThreadData()
//{
//
//	// TODO : only create command pool when requested
//	for (auto& thread : mThreadData)
//	{
//		//thread.commandPool = std::make_unique<CommandPool>(mDevice, mDevice.queueFamilyIndices().graphicsFamily);
//	}
//}

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

//void Frame::createSynchronisation()
//{
//	// Fence creation information
//	VkFenceCreateInfo fenceCreateInfo = {};
//	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;			// Create fence as signaled (open)
//
//	// Semaphore creation information
//	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
//	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
//
//	if (vkCreateSemaphore(mDevice.logicalDevice(), &semaphoreCreateInfo, nullptr, &mImageAvailable) != VK_SUCCESS ||
//		vkCreateSemaphore(mDevice.logicalDevice(), &semaphoreCreateInfo, nullptr, &mRenderFinished) != VK_SUCCESS ||
//		vkCreateFence(mDevice.logicalDevice(), &fenceCreateInfo, nullptr, &mDrawFence) != VK_SUCCESS)
//	{
//		throw std::runtime_error("Failed to create a Semaphore and/or Fence!");
//	}
//}
