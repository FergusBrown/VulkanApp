#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <memory>

#include "Device.h"

class Swapchain
{
public:

private:
	Device& mDevice;

	VkSwapchainKHR mSwapchain;

	// - Swapchain details
	VkSurfaceCapabilitiesKHR surfaceCapabilities;		// Surface properties e.g. image size/extent
	std::vector<VkSurfaceFormatKHR> mFormats;			// Surface image formats e.g. RGBA and size of each
	std::vector<VkPresentModeKHR> mPresentationModes;	// How images should be presented to screen
	
	// - Presentation mode priority  (vector[0] has high priority, vector[size-1] has low priority)
	std::vector<VkPresentModeKHR> mPresentationModePriority = {
		VK_PRESENT_MODE_MAILBOX_KHR,				// Prefer triple buffer
		VK_PRESENT_MODE_FIFO_RELAXED_KHR };			// Then relaxed double buffer

	VkSurfaceKHR mSurface;
	

	// - Images
	std::vector<VkImage> mImages;
};

