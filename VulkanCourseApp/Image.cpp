#include "Image.h"

Image::Image(Device& device, const VkExtent2D& extent, uint32_t mipLevels, uint32_t arrayLayerCount, VkFormat format, VkImageTiling tiling, VkSampleCountFlagBits sampleCount, VkImageUsageFlags usage, VkMemoryPropertyFlags propFlags, VkImageAspectFlags aspectMask) :
	mDevice(device), mExtent({ extent.width, extent.height, 1 }), mSubresource({aspectMask, mipLevels, arrayLayerCount}), mFormat(format), mTiling(tiling), mSampleCount(sampleCount), mUsage(usage), mPropFlags(propFlags)
{
	// CREATE IMAGE
	// Image Creation Info
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;					// Type of image (1D, 2D, or 3D)
	imageCreateInfo.extent.width = extent.width;					// Depth of image extent
	imageCreateInfo.extent.height = extent.height;					// Height of image extent
	imageCreateInfo.extent.depth = 1;								// Depth of image (just 1, no 3D aspect)
	imageCreateInfo.mipLevels = mipLevels;									// Number of mipmap levels
	imageCreateInfo.arrayLayers = arrayLayerCount;					// Number of levels in image array
	imageCreateInfo.format = format;								// Format type of image
	imageCreateInfo.tiling = tiling;								// How image data should be "tiled" (arranged for optimal reading)
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		// Layout of image data on creation
	imageCreateInfo.usage = usage;									// Bit flags defining what image will be used for
	imageCreateInfo.samples = mSampleCount;							// Number of samples for multi-sampling
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;		// Whether image can be shared between queues

	// Create image
	VkResult result = vkCreateImage(mDevice.logicalDevice(), &imageCreateInfo, nullptr, &mImage);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create an Image!");
	}

	// CREATE MEMORY FOR IMAGE
	createMemory();
	createImageView();
}

Image::~Image()
{
	vkDestroyImageView(mDevice.logicalDevice(), mImageView, nullptr);
	vkDestroyImage(mDevice.logicalDevice(), mImage, nullptr);
	vkFreeMemory(mDevice.logicalDevice(), mMemory, nullptr);
}

const VkImage& Image::image()
{
	return mImage;
}

const VkImageView& Image::imageView()
{
	return mImageView;
}

void Image::createImageView()
{
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = mImage;										// Image to create view for
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;					// Type of image (1D, 2D, 3D etc.)
	viewCreateInfo.format = mFormat;										// Format of image data
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;		// Allows remapping of rgba components to other rgba values
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	// Subresources allow the view to view only a part of a image
	viewCreateInfo.subresourceRange.aspectMask = mSubresource.aspectMask;			// Which aspect of image to view (e.g. COLOR_BIT for viewing color)
	viewCreateInfo.subresourceRange.baseMipLevel = 0;					// Start mipmap level to view from
	viewCreateInfo.subresourceRange.levelCount = mSubresource.mipLevel;						// Number of mipmap levels to view
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;					// Starrt array level to view from
	viewCreateInfo.subresourceRange.layerCount = mSubresource.arrayLayer;						// Number of array levels to view

	// Create Image View
	VkResult result = vkCreateImageView(mDevice.logicalDevice(), &viewCreateInfo, nullptr, &mImageView);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create an Image View!");
	}
}

void Image::createMemory()
{
	// Get memory requirements for a type of image
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(mDevice.logicalDevice(), mImage, &memoryRequirements);

	// Allocate memory using image requirements and user defined properties
	VkMemoryAllocateInfo memoryAllocInfo = {};
	memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocInfo.allocationSize = memoryRequirements.size;
	memoryAllocInfo.memoryTypeIndex = findMemoryTypeIndex(mDevice.physicalDevice(), memoryRequirements.memoryTypeBits, mPropFlags);

	VkResult result = vkAllocateMemory(mDevice.logicalDevice(), &memoryAllocInfo, nullptr, &mMemory);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate memory for image!");
	}

	// Connect memory to image
	vkBindImageMemory(mDevice.logicalDevice(), mImage, mMemory, 0);
}
