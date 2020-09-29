#include "SemaphorePool.h"

#include "Device.h"

SemaphorePool::SemaphorePool(Device& device) :
	mDevice(device)
{
}

SemaphorePool::~SemaphorePool()
{
	reset();

	for (VkSemaphore semaphore : mSemaphores)
	{
		vkDestroySemaphore(mDevice.logicalDevice(), semaphore, nullptr);
	}

	mSemaphores.clear();
}

VkSemaphore SemaphorePool::requestSemaphore()
{
	if (mActiveSemaphores < mSemaphores.size())
	{
		// Return semaphore the existing semaphore and increment active number of semaphores
		return mSemaphores.at(mActiveSemaphores++);
	}

	// Create and return a semaphore handle
	VkSemaphore semaphore{ VK_NULL_HANDLE };
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkResult result = vkCreateSemaphore(mDevice.logicalDevice(), &semaphoreCreateInfo, nullptr, &semaphore);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Could not create a Semaphore!");
	}

	mSemaphores.push_back(semaphore);

	++mActiveSemaphores;

	return semaphore;
}

void SemaphorePool::reset()
{
	mActiveSemaphores = 0;
}
