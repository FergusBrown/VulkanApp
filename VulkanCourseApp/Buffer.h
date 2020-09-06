#pragma once
#include "Common.h"

class Buffer
{
public:

	// - Getters
	VkBuffer handle() const;
private:
	VkBuffer mHandle{ VK_NULL_HANDLE };
};

