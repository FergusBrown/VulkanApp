#pragma once
#include "Common.h"
#include "Utilities.h"
#include "Device.h"

// TODO: add functionality to change image properties based on member variables
// TODO: support for 3D images
// Container for an image and the image view referring to this image
class Image
{
public:
	Image(Device& device,
		const VkExtent2D& extent,
		uint32_t mipLevels,
		uint32_t arrayLayerCount,
		VkFormat format,
		VkImageTiling tiling,
		VkSampleCountFlagBits sampleCount,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags propFlags,
		VkImageAspectFlags aspectMask
	);
	~Image();

	// - Getters
	Device& device() const;
	const VkImage& image() const;
	const VkImageView& imageView() const;
	const VkExtent3D& extent() const;
	VkFormat format() const;
	VkSampleCountFlagBits sampleCount() const;
	VkImageUsageFlags usage() const;

private:
	Device& mDevice;
	VkImage mImage;

	// - Image attributes
	VkImageType mType;
	VkExtent3D mExtent;		// use different constructor if creating a 3D image
	VkImageSubresource mSubresource;
	VkFormat mFormat;
	VkImageTiling mTiling;
	VkSampleCountFlagBits mSampleCount;
	VkMemoryPropertyFlags mPropFlags;
	VkImageUsageFlags mUsage;
	
	// - Associated with image
	VkDeviceMemory mMemory;
	VkImageView mImageView;

	// - Image view management
	void createImageView();

	// - Memory management
	void createMemory();
};

