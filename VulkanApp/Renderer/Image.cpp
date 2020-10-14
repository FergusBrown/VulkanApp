#include "Image.h"

#include "Device.h"
#include "DeviceMemory.h"

// Create image from an existing image handle (e.g. swapchain image)
Image::Image(Device& device, 
	VkImage image, 
	const VkExtent2D& extent, 
	VkFormat format, 
	VkImageUsageFlags usage,
	VkSampleCountFlagBits sampleCount) :

	mDevice(device),
	mHandle(image),
	mExtent({ extent.width, extent.height, 1 }),
	mFormat(format),
	mSampleCount(sampleCount),
	mUsage(usage)
{
	mSubresource.arrayLayer = 1;
	mSubresource.mipLevel = 1;
	//mSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	//createImageView();
}

Image::Image(Device& device, 
	const VkExtent2D& extent, 
	VkFormat format, 
	VkImageUsageFlags usage, 
	VkMemoryPropertyFlags propFlags, 
	//VkImageAspectFlags aspectMask,
	VkSampleCountFlagBits sampleCount,
	uint32_t mipLevels,
	uint32_t arrayLayerCount,
	VkImageTiling tiling,
	VkImageLayout initialLayout) :

	mDevice(device), 
	mType(VK_IMAGE_TYPE_2D),
	mExtent({ extent.width, extent.height, 1 }), 
	//mSubresource({aspectMask, mipLevels, arrayLayerCount}), 
	mFormat(format), 
	mTiling(tiling), 
	mSampleCount(sampleCount), 
	mUsage(usage), 
	//mPropFlags(propFlags),
	mSharingMode(VK_SHARING_MODE_EXCLUSIVE),
	mLayout(initialLayout)
{
	mSubresource.mipLevel = mipLevels;
	mSubresource.arrayLayer = arrayLayerCount;

	// CREATE IMAGE
	createImage();

	// CREATE DEVICE MEMORY
	createMemory(propFlags);

	// CREATE VIEW
	//createImageView();
}

//Image::Image(Device& device, 
//	const VkExtent2D& extent, 
//	VkFormat format, 
//	VkImageUsageFlags usage, 
//	VkMemoryPropertyFlags propFlags, 
//	VkImageAspectFlags aspectMask, VkImageLayout initialLayout, 
//	VkSampleCountFlagBits sampleCount, 
//	uint32_t mipLevels, 
//	uint32_t arrayLayerCount, 
//	VkImageTiling tiling) :
//
//	mDevice(device),
//	mType(VK_IMAGE_TYPE_2D),
//	mExtent({ extent.width, extent.height, 1 }),
//	mSubresource({ aspectMask, mipLevels, arrayLayerCount }),
//	mFormat(format),
//	mTiling(tiling),
//	mSampleCount(sampleCount),
//	mUsage(usage),
//	//mPropFlags(propFlags),
//	mSharingMode(VK_SHARING_MODE_EXCLUSIVE),
//	mLayout(initialLayout)
//{
//	// CREATE IMAGE
//	createImage();
//
//	// CREATE DEVICE MEMORY
//	createMemory(propFlags);
//
//	// CREATE VIEW
//	createImageView();
//}

Image::Image(Image&& other) :
	mDevice(other.mDevice),
	mType(other.mType),
	mExtent(other.mExtent),
	mSubresource(other.mSubresource),
	mFormat(other.mFormat),
	mTiling(other.mTiling),
	mSampleCount(other.mSampleCount),
	mUsage(other.mUsage),
	//mPropFlags(propFlags),
	mSharingMode(other.mSharingMode),
	mLayout(other.mLayout),
	mHandle(other.mHandle),
	mMemory(std::move(other.mMemory))
	//mImageView(other.mImageView)
{
	other.mHandle = VK_NULL_HANDLE;
}

Image::~Image()
{
	

	// If memory nullptr then object was created from an external VkImage (from swapchain)
	// Therefore, only destroy image if it is not from swapchain
	if (mHandle != VK_NULL_HANDLE && mMemory)
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

//const VkImageView& Image::imageView() const
//{
//	return mImageView;
//}

const VkExtent3D& Image::extent() const
{
	return mExtent;
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

VkImageLayout Image::layout() const
{
	return mLayout;
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
	imageCreateInfo.mipLevels = mSubresource.mipLevel;				// Number of mipmap levels
	imageCreateInfo.arrayLayers = mSubresource.arrayLayer;			// Number of levels in image array
	imageCreateInfo.format = mFormat;								// Format type of image
	imageCreateInfo.tiling = mTiling;								// How image data should be "tiled" (arranged for optimal reading)
	imageCreateInfo.initialLayout = mLayout;						// Layout of image data on creation
	imageCreateInfo.usage = mUsage;									// Bit flags defining what image will be used for
	imageCreateInfo.samples = mSampleCount;							// Number of samples for multi-sampling
	imageCreateInfo.sharingMode = mSharingMode;						// Whether image can be shared between queues

	// Create image
	VkResult result = vkCreateImage(mDevice.logicalDevice(), &imageCreateInfo, nullptr, &mHandle);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create an Image!");
	}
}
