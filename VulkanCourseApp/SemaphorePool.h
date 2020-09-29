#pragma once
#include "Common.h"

class Device;

class SemaphorePool
{
public:
	SemaphorePool(Device& device);
	SemaphorePool() = delete;
	~SemaphorePool();

	SemaphorePool(const SemaphorePool&) = delete;
	SemaphorePool(SemaphorePool&&) = delete;

	VkSemaphore requestSemaphore();
	void reset();
private:
	Device& mDevice;

	std::vector<VkSemaphore> mSemaphores;

	uint32_t mActiveSemaphores{ 0 };
};

