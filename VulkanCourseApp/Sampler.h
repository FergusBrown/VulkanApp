#pragma once
#include "Common.h"

class Sampler
{
public:

	// - Getters
	VkSampler handle() const;
private:
	VkSampler mHandle{ VK_NULL_HANDLE };

};

