#pragma once
#include "Common.h"

class Device;

class Pipeline
{
public:

	~Pipeline();

	// - Getters
	VkPipeline handle() const;

private:
	Device& mDevice;

	VkPipeline mHandle{VK_NULL_HANDLE};

};

