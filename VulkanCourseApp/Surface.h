#pragma once
#include "Common.h"

class Instance;

// An object to contain the surface handle
// The main purpose of this class is to ensure that destruction of vulkan handles happens in the correct order 
// (e.g. after swpachain but before instance destruction)
class Surface
{
public:
	Surface(Instance& instance, VkSurfaceKHR handle);
	~Surface();

	VkSurfaceKHR handle() const;
private:
	Instance& mInstance;

	VkSurfaceKHR mHandle{ VK_NULL_HANDLE };
};

