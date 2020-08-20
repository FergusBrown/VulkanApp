#include "SwapChain.h"

Swapchain::Swapchain(Device& device, const VkExtent2D& newExtent) :
	mDevice(device)
{
	createSwapchain(newExtent);
}

Swapchain::~Swapchain()
{
	vkDestroySwapchainKHR(mDevice.logicalDevice(), mSwapchain, nullptr);
}

VkSwapchainKHR Swapchain::swapchain() const
{
	return mSwapchain;
}

const SwapchainDetails& Swapchain::details() const
{
	return mDetails;
}

const std::vector<VkImage> Swapchain::images() const
{
	return mImages;
}

void Swapchain::createSwapchain(const VkExtent2D& newExtent)
{

	// Get surface support so that we can pick best settings
	SurfaceSupport surfaceSupport;
	getSurfaceSupport(surfaceSupport);

	// 1. CHOOSE BEST SURFACE FORMAT
	chooseSurfaceFormat(surfaceSupport.formats);
	// 2. CHOOSE BEST PRESENTATION MODE
	choosePresentationMode(surfaceSupport.presentationModes);
	// 3. CHOOSE SWAP CHAIN IMAGE RESOLUTION
	chooseExtent(surfaceSupport.surfaceCapabilities, newExtent);

	// How many images are in the swap chain? Get 1 more than the minimum to allow triple buffering
	mDetails.imageCount = surfaceSupport.surfaceCapabilities.minImageCount + 1;

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
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;						// What attachment images will be used at
	swapChainCreateInfo.preTransform = surfaceSupport.surfaceCapabilities.currentTransform;		// Transform to perform on swap chain
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;						// How to handle blending images with external graphics (e.g. other windows)
	swapChainCreateInfo.clipped = VK_TRUE;														// whether to clip part of images not in view (e.g. behind another window, off screen etc.)

	// Get queue family indices
	QueueFamilyIndices indices = mDevice.queueFamilyIndices();

	// If Graphics and Presentation families are different, then swapchain must let images be shared between families
	if (indices.graphicsFamily != indices.presentationFamily)
	{
		// Queues to share between
		uint32_t queueFamilyIndices[] = {
			(uint32_t)indices.graphicsFamily,
			(uint32_t)indices.presentationFamily
		};

		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;	// Image share handling
		swapChainCreateInfo.queueFamilyIndexCount = 2;						// Number of queues to share images between -- 2 in this case as must be shared between graphics and presentation queue
		swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;		// Array of queues to share between
	}
	else
	{
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	// If old swapchain been destroyed and this one replaces it, then link old one to quickly hand over responsibilities
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	// Create Swapchain
	VkResult result = vkCreateSwapchainKHR(mDevice.logicalDevice(), &swapChainCreateInfo, nullptr, &mSwapchain);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Swapchain!");
	}

	// get Swapchain images (first count, then values)
	uint32_t swapChainImageCount;
	vkGetSwapchainImagesKHR(mDevice.logicalDevice(), mSwapchain, &swapChainImageCount, nullptr);
	mImages.resize(swapChainImageCount);
	vkGetSwapchainImagesKHR(mDevice.logicalDevice(), mSwapchain, &swapChainImageCount, mImages.data());

	// TODO: move swap chain image views to render target
	/*for (VkImage image : images)
	{
		// Store image handle
		SwapchainImage swapChainImage = {};
		swapChainImage.image = image;
		swapChainImage.imageView = createImageView(image, swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

		// add to swap chain image list
		swapChainImages.push_back(swapChainImage);
	}*/
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

void Swapchain::choosePresentationMode(std::vector<VkPresentModeKHR>& presentationModes)
{
	// Look for Mailbox presentation mode
	for (const auto& presentationMode : presentationModes)
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
	return;
}

void Swapchain::chooseExtent(VkSurfaceCapabilitiesKHR& surfaceCapabilities, const VkExtent2D& newExtent)
{
	VkExtent2D currentExtent = surfaceCapabilities.currentExtent;

	// If current extent is at numeric limits, then extent can vary. Otherwise, it is the size of the window
	if (currentExtent.width != std::numeric_limits <uint32_t>::max())
	{
		mDetails.extent = currentExtent;
	}
	else
	{
		// If value can vary, clamp the passed in value and use that
		mDetails.extent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width));
		mDetails.extent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, newExtent.height));
	}
}
