#include "Swapchain.h"

#include "Device.h"

// TODO: check usage is valid
Swapchain::Swapchain(Device& device, 
	const VkExtent2D& extent, 
	VkSurfaceKHR surface,
	VkPresentModeKHR presentMode,
	VkImageUsageFlags usage) :
	mDevice(device),
	mSurface(surface)
{
	mDetails.usage = usage;

	// Get surface support so that we can choose supported/best settings
	SurfaceSupport surfaceSupport;
	getSurfaceSupport(surfaceSupport);

	// 1. CHOOSE BEST SURFACE FORMAT
	chooseSurfaceFormat(surfaceSupport.formats);
	// 2. CHOOSE BEST PRESENTATION MODE
	choosePresentationMode(surfaceSupport.presentationModes, presentMode);
	// 3. CHOOSE NUMBER OF IMAGES
	chooseImageCount();
	// 4. CHOOSE SWAP CHAIN IMAGE RESOLUTION
	chooseExtent(surfaceSupport.surfaceCapabilities, extent);

	// If imageCount higher than max, then clamp down to max
	// If 0 then limitless
	if (surfaceSupport.surfaceCapabilities.maxImageCount > 0
		&& surfaceSupport.surfaceCapabilities.maxImageCount < mDetails.imageCount)
	{
		mDetails.imageCount = surfaceSupport.surfaceCapabilities.maxImageCount;
	}

	// Creation information for swap chain
	VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface = mSurface;														// Swapchain surface
	swapChainCreateInfo.imageFormat = mDetails.surfaceFormat.format;							// Swapchain format
	swapChainCreateInfo.imageColorSpace = mDetails.surfaceFormat.colorSpace;					// Swapchain color space
	swapChainCreateInfo.presentMode = mDetails.presentMode;										// Swapchain presentation mode
	swapChainCreateInfo.imageExtent = mDetails.extent;											// Swapchain image extents
	swapChainCreateInfo.minImageCount = mDetails.imageCount;									// Minimum images in swapchain
	swapChainCreateInfo.imageArrayLayers = 1;													// Number of layers for each image in chain
	swapChainCreateInfo.imageUsage = mDetails.usage;						// What attachment images will be used at
	swapChainCreateInfo.preTransform = surfaceSupport.surfaceCapabilities.currentTransform;		// Transform to perform on swap chain
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;						// How to handle blending images with external graphics (e.g. other windows)
	swapChainCreateInfo.clipped = VK_TRUE;														// whether to clip part of images not in view (e.g. behind another window, off screen etc.)
	swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapChainCreateInfo.pQueueFamilyIndices = nullptr;

	// If old swapchain been destroyed and this one replaces it, then link old one to quickly hand over responsibilities
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	// Create Swapchain
	VkResult result = vkCreateSwapchainKHR(mDevice.logicalDevice(), &swapChainCreateInfo, nullptr, &mHandle);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Swapchain!");
	}

	// get Swapchain images (first count, then values)
	uint32_t swapChainImageCount;
	vkGetSwapchainImagesKHR(mDevice.logicalDevice(), mHandle, &swapChainImageCount, nullptr);
	mImages.resize(swapChainImageCount);
	vkGetSwapchainImagesKHR(mDevice.logicalDevice(), mHandle, &swapChainImageCount, mImages.data());
}

Swapchain::~Swapchain()
{
	vkDestroySwapchainKHR(mDevice.logicalDevice(), mHandle, nullptr);
}

Device& Swapchain::device() const
{
	return mDevice;
}

VkSwapchainKHR Swapchain::handle() const
{
	return mHandle;
}

const SwapchainDetails& Swapchain::details() const
{
	return mDetails;
}

const VkExtent2D& Swapchain::extent() const
{
	return mDetails.extent;
}

VkFormat Swapchain::format() const
{
	return mDetails.surfaceFormat.format;
}

VkImageUsageFlags Swapchain::usage() const
{
	return mDetails.usage;
}

const std::vector<VkImage>& Swapchain::images() const
{
	return mImages;
}

void Swapchain::setPresentationPriority(const std::vector<VkPresentModeKHR>& newList)
{
	mPresentationModePriority = newList;
}

void Swapchain::setSurfaceFormatPriority(const std::vector<VkSurfaceFormatKHR>& newList)
{
	mSurfaceFormatPriorityList = newList;
}


//	NOTE: this function will only return once the fence and semaphore are signalled
VkResult Swapchain::acquireNextImage(VkFence drawFence, VkSemaphore imageAvailable, uint32_t& imageIndex)
{
	// GET NEXT IMAGE
	// 1. Wait for fence to be signalled and reset it
	// 2. Use vkAcquireNextImageKHR get the appropriate image index


	// Wait for given fence to signal (open) from last draw before continuing
	vkWaitForFences(mDevice.logicalDevice(), 1, &drawFence, VK_TRUE, std::numeric_limits<uint64_t>::max());		// ANALOGY : Wait until this fence is open (freezes the code)
	// Manually reset (close) fences
	vkResetFences(mDevice.logicalDevice(), 1, &drawFence);

	return vkAcquireNextImageKHR(mDevice.logicalDevice(), mHandle, std::numeric_limits<uint64_t>::max(), imageAvailable, VK_NULL_HANDLE, &imageIndex);;
}

void Swapchain::getSurfaceSupport(SurfaceSupport& surfaceSupport)
{
	VkPhysicalDevice device = mDevice.physicalDevice();

	// -- CAPABILITIES --
	// Get the surface capabilities for the given surface on the given physical device
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mSurface, &surfaceSupport.surfaceCapabilities);

	// -- FORMATS --
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, nullptr);

	// If formats returned, get list of formats
	if (formatCount != 0)
	{
		surfaceSupport.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, surfaceSupport.formats.data());
	}

	// -- PRESENTATION MODES --
	uint32_t presentationCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentationCount, nullptr);

	// If presentation modes returned, get list of presentation mdoes
	if (presentationCount != 0)
	{
		surfaceSupport.presentationModes.resize(presentationCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentationCount, surfaceSupport.presentationModes.data());
	}
}

void Swapchain::chooseSurfaceFormat(std::vector<VkSurfaceFormatKHR>& formats)
{
	// If only one format available and is undefined, then this means ALL formats are available (no restrictions)
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		mDetails.surfaceFormat = mSurfaceFormatPriorityList[0];
		return;
	}

	// If restricted seatch for a format on the priority list
	for (const auto& format : formats)
	{
		for (size_t i = 0; i < mSurfaceFormatPriorityList.size(); ++i)
		{
			if (mSurfaceFormatPriorityList[i].format == format.format
				&& mSurfaceFormatPriorityList[i].colorSpace == format.colorSpace)
			{
				mDetails.surfaceFormat = mSurfaceFormatPriorityList[i];
				return;
			}
		}
	}

	// If can't find optimal format, then just set first format
	mDetails.surfaceFormat = formats[0];
}

// If requested mode is supported then set it. Otherwise find the highest priority presentation mode available
void Swapchain::choosePresentationMode(std::vector<VkPresentModeKHR>& supportedPresentationModes, VkPresentModeKHR requestedPresentationMode)
{
	// Look for requested presentation mode
	for (const auto& presentationMode : supportedPresentationModes)
	{
		if (presentationMode == requestedPresentationMode)
		{
			mDetails.presentMode = requestedPresentationMode;
			return;
		}
	}

	// Otherwise set to a supported presentation mode in the priority list
	for (const auto& presentationMode : supportedPresentationModes)
	{
		for (size_t i = 0; i < mPresentationModePriority.size(); ++i)
		{
			if (presentationMode == mPresentationModePriority[i])
			{
				mDetails.presentMode = mPresentationModePriority[i];
				return;
			}
		}
	}

	// make this the default as it is required as part of vulkan spec so must be present
	mDetails.presentMode = VK_PRESENT_MODE_FIFO_KHR;
}

void Swapchain::chooseImageCount()
{
	// TODO : these values should be checked against the surfaceCapabilities to ensure they work
	if (mDetails.presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
	{
		mDetails.imageCount = IMAGE_COUNT_MAILBOX;
	}
	else
	{
		mDetails.imageCount = IMAGE_COUNT_DEFAULT;
	}
}

// If requested extent not supported then clamp the extent values to a supported extent
void Swapchain::chooseExtent(VkSurfaceCapabilitiesKHR& supportedSurfaceCapabilities, const VkExtent2D& requestedExtent)
{
	VkExtent2D currentExtent = supportedSurfaceCapabilities.currentExtent;

	// If current extent is at numeric limits, then extent can vary. Otherwise, it is the size of the window
	if (currentExtent.width != std::numeric_limits <uint32_t>::max())
	{
		mDetails.extent = currentExtent;
	}
	else
	{
		// If value can vary, clamp the passed in value and use that
		mDetails.extent.width = std::max(supportedSurfaceCapabilities.minImageExtent.width, std::min(supportedSurfaceCapabilities.maxImageExtent.width, requestedExtent.width));
		mDetails.extent.height = std::max(supportedSurfaceCapabilities.minImageExtent.height, std::min(supportedSurfaceCapabilities.maxImageExtent.height, requestedExtent.height));
	}
}
