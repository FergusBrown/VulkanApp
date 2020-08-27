#include "Frame.h"

Frame::Frame(Device& device, std::unique_ptr<RenderTarget>& renderTarget, uint32_t threadCount) :
	mDevice(device), mRenderTarget(renderTarget), mThreadCount(threadCount)
{
	createThreadPool();

	// CREATE DESCRIPTOR SETS

	// CREATE COMMAND BUFFERS


}

Frame::~Frame()
{
}

Device& Frame::device() const
{
	return mDevice;
}


void Frame::createThreadPool()
{
	mThreadCount = std::thread::hardware_concurrency();
	mThreadPool.resize(mThreadCount);
	mThreadData.resize(mThreadCount);

	for (auto& thread : mThreadData)
	{
		thread.secondaryCommandBuffers.resize(mThreadCount);
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
