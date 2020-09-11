#include "Image.h"

// TODO : unfinished constructor
Image::Image(Device& device, 
	VkImage image, 
	const VkExtent2D& extent, 
	VkFormat format, 
	VkImageUsageFlags usage,
	VkSampleCountFlagBits sampleCount) :

	mDevice(device),
	mExtent({ extent.width, extent.height, 1 }),
	mFormat(format),
	mSampleCount(sampleCount),
	mUsage(usage)
{
	mSubresource.arrayLayer = 1;
	mSubresource.mipLevel = 1;
	mSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	createImageView();
}

Image::Image(Device& device, 
	const VkExtent2D& extent, 
	VkFormat format, 
	VkImageUsageFlags usage, 
	VkMemoryPropertyFlags propFlags, 
	VkImageAspectFlags aspectMask,
	VkSampleCountFlagBits sampleCount,
	uint32_t mipLevels,
	uint32_t arrayLayerCount,
	VkImageTiling tiling,
	VkImageLayout initialLayout) :

	mDevice(device), 
	mType(VK_IMAGE_TYPE_2D),
	mExtent({ extent.width, extent.height, 1 }), 
	mSubresource({aspectMask, mipLevels, arrayLayerCount}), 
	mFormat(format), 
	mTiling(tiling), 
	mSampleCount(sampleCount), 
	mUsage(usage), 
	//mPropFlags(propFlags),
	mSharingMode(VK_SHARING_MODE_EXCLUSIVE),
	mLayout(initialLayout)
{
	// CREATE IMAGE
	createImage();

	// CREATE DEVICE MEMORY
	createMemory(propFlags);

	// CREATE VIEW
	createImageView();
}

Image::Image(Device& device, 
	const VkExtent2D& extent, 
	VkFormat format, 
	VkImageUsageFlags usage, 
	VkMemoryPropertyFlags propFlags, 
	VkImageAspectFlags aspectMask, VkImageLayout initialLayout, 
	VkSampleCountFlagBits sampleCount, 
	uint32_t mipLevels, 
	uint32_t arrayLayerCount, 
	VkImageTiling tiling) :

	mDevice(device),
	mType(VK_IMAGE_TYPE_2D),
	mExtent({ extent.width, extent.height, 1 }),
	mSubresource({ aspectMask, mipLevels, arrayLayerCount }),
	mFormat(format),
	mTiling(tiling),
	mSampleCount(sampleCount),
	mUsage(usage),
	//mPropFlags(propFlags),
	mSharingMode(VK_SHARING_MODE_EXCLUSIVE),
	mLayout(initialLayout)
{
	// CREATE IMAGE
	createImage();

	// CREATE DEVICE MEMORY
	createMemory(propFlags);

	// CREATE VIEW
	createImageView();
}

Image::~Image()
{
	vkDestroyImageView(mDevice.logicalDevice(), mImageView, nullptr);

	// If memory null then object was created from an exteernal VkImage
	if (mHandle != VK_NULL_HANDLE)
	{
		vkDestroyImage(mDevice.logicalDevice(), mHandle, nullptr);
		
	}
		
}

Device& Image::device() const
{
	return mDevice;
}

const VkImage& Image::handle() const
{
	return mHandle;
}

const VkImageView& Image::imageView() const
{
	return mImageView;
}

const VkExtent3D& Image::extent() const
{
	return extent();
}

const VkImageSubresource& Image::subresource() const
{
	return mSubresource;
}

VkFormat Image::format() const
{
	return mFormat;
}

VkSampleCountFlagBits Image::sampleCount() const
{
	return mSampleCount;
}

VkImageUsageFlags Image::usage() const
{
	return mUsage;
}

VkDeviceMemory Image::memory() const
{
	return mMemory->handle();
}

void Image::createImageView()
{
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = mHandle;										// Image to create view for
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;					// Type of image (1D, 2D, 3D etc.)
	viewCreateInfo.format = mFormat;										// Format of image data
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;		// Allows remapping of rgba components to other rgba values
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	// Subresources allow the view to view only a part of a image
	viewCreateInfo.subresourceRange.aspectMask = mSubresource.aspectMask;			// Which aspect of image to view (e.g. COLOR_BIT for viewing color)
	viewCreateInfo.subresourceRange.baseMipLevel = 0;								// Start mipmap level to view from
	viewCreateInfo.subresourceRange.levelCount = mSubresource.mipLevel;				// Number of mipmap levels to view
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;								// Starrt array level to view from
	viewCreateInfo.subresourceRange.layerCount = mSubresource.arrayLayer;			// Number of array levels to view

	// Create Image View
	VkResult result = vkCreateImageView(mDevice.logicalDevice(), &viewCreateInfo, nullptr, &mImageView);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create an Image View!");
	}
}

void Image::createMemory(VkMemoryPropertyFlags propFlags)
{
	// Get memory requirements for a type of image
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(mDevice.logicalDevice(), mHandle, &memoryRequirements);

	mMemory = std::make_unique<DeviceMemory>(mDevice, propFlags, memoryRequirements);

	// Connect memory to image
	vkBindImageMemory(mDevice.logicalDevice(), mHandle, mMemory->handle(), 0);
}

void Image::createImage()
{
	// CREATE IMAGE
	// Image Creation Info
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = mType;								// Type of image (1D, 2D, or 3D)
	imageCreateInfo.extent.width = mExtent.width;					// Depth of image extent
	imageCreateInfo.extent.height = mExtent.height;					// Height of image extent
	imageCreateInfo.extent.depth = mExtent.depth;					// Depth of image (just 1, no 3D aspect)
	imageCreateInfo.mipLevels = mSubresource.mipLevel;							// Number of mipmap levels
	imageCreateInfo.arrayLayers = mSubresource.arrayLayer;						// Number of levels in image array
	imageCreateInfo.format = mFormat;								// Format type of image
	imageCreateInfo.tiling = mTiling;								// How image data should be "tiled" (arranged for optimal reading)
	imageCreateInfo.initialLayout = mLayout;					// Layout of image data on creation
	imageCreateInfo.usage = mUsage;									// Bit flags defining what image will be used for
	imageCreateInfo.samples = mSampleCount;								// Number of samples for multi-sampling
	imageCreateInfo.sharingMode = mSharingMode;						// Whether image can be shared between queues

	// Create image
	VkResult result = vkCreateImage(mDevice.logicalDevice(), &imageCreateInfo, nullptr, &mHandle);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create an Image!");
	}
}
