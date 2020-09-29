#pragma once
#include "Common.h"

class Device;

// An object to provide fence handles upon request
// wait function will wait for all active fences to be signalled
// reset will reset all fences and set active
class FencePool
{
public:
	FencePool(Device& device);
	FencePool() = delete;
	~FencePool();

	FencePool(const FencePool&) = delete;
	FencePool(FencePool&&) = delete;

	VkFence requestFence();
	VkResult reset();
	VkResult wait(uint32_t timeout = std::numeric_limits<uint32_t>::max()) const;

private:
	Device& mDevice;

	std::vector<VkFence> mFences;

	uint32_t mActiveFences{ 0 };
};

