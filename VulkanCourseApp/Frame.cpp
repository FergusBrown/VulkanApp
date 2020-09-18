#include "Frame.h"

Frame::Frame(Device& device, std::unique_ptr<RenderTarget>&& renderTarget, uint32_t threadCount) :
	mDevice(device), mRenderTarget(std::move(renderTarget)), mThreadCount(threadCount)
{
	mThreadPool.resize(mThreadCount);
	mThreadData.resize(mThreadCount);

	// THREAD SPECIFIC DATA : COMMAND + DESCRIPTOR POOLS
	//createThreadData();

}

Frame::~Frame()
{
}

Device& Frame::device() const
{
	return mDevice;
}

//CommandPool& Frame::commandPool(uint32_t threadIndex)
//{
//	return *mThreadData[threadIndex].commandPools;
//}

void Frame::reset()
{
	for (auto& thread : mThreadData)
	{
		thread.commandPool->reset();
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
std::unique_ptr<CommandPool>& Frame::commandPool(const Queue& queue, uint32_t threadIndex)
{
	auto& commandPools = mThreadData[threadIndex].commandPools;

	for (auto& pool : commandPools)
	{

	}
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
