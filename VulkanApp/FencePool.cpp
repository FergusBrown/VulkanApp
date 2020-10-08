#include "FencePool.h"

#include "Device.h"

FencePool::FencePool(Device& device) :
	mDevice(device)
{
}

FencePool::~FencePool()
{
	wait();
	reset();

	for (VkFence fence : mFences)
	{
		vkDestroyFence(mDevice.logicalDevice(), fence, nullptr);
	}

	mFences.clear();
}

// Provides an existing fence if available and otherwise creates a new fence
VkFence FencePool::requestFence()
{
	if (mActiveFences < mFences.size())
	{
		// Return the existing fence and increment active number of fences
		return mFences.at(mActiveFences++);
	}

	// Create and return a fence handle
	VkFence fence{ VK_NULL_HANDLE };
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	//fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;			// Create fence as signaled (open)

	VkResult result = vkCreateFence(mDevice.logicalDevice(), &fenceCreateInfo, nullptr, &fence);

	if(result != VK_SUCCESS)
	{
		throw std::runtime_error("Could not create a Fence!");
	}

	mFences.push_back(fence);

	++mActiveFences;

	return fence;
}

VkResult FencePool::reset()
{
	if (mFences.empty() || mActiveFences == 0)
	{
		return VK_SUCCESS;
	}

	VkResult result = vkResetFences(mDevice.logicalDevice(), mFences.size(), mFences.data());

	if (result != VK_SUCCESS)
	{
		return result;
	}

	mActiveFences = 0;

	return result;
}

VkResult FencePool::wait(uint32_t timeout) const
{
	if (mFences.empty() || mActiveFences == 0)
	{
		return VK_SUCCESS;
	}

	return vkWaitForFences(mDevice.logicalDevice(), mFences.size(), mFences.data(), VK_TRUE, timeout);
}
