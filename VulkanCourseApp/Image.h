#pragma once
#include "Common.h"
#include "Utilities.h"
#include "Device.h"
#include "DeviceMemory.h"

// TODO: add functionality to change image properties based on member variables
// TODO: support for 3D images
// Container for an image and the image view referring to this image
class Image
{
public:
	// Create an Image object from an existing handle e.g. Swapchain image
	Image(Device& device,
		VkImage image,
		const VkExtent2D& extent,
		VkFormat format,
		VkImageUsageFlags usage,
		VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT
	);

	// Create an image object from input parameters
	Image(Device& device,
		const VkExtent2D& extent,
		VkFormat format,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags propFlags,
		VkImageAspectFlags aspectMask,
		VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
		uint32_t mipLevels = 1,
		uint32_t arrayLayerCount = 1,
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
		VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	);

	//// Same as above, different order of arguments
	//Image(Device& device,
	//	const VkExtent2D& extent,
	//	VkFormat format,
	//	VkImageUsageFlags usage,
	//	VkMemoryPropertyFlags propFlags,
	//	VkImageAspectFlags aspectMask,
	//	VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	//	VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
	//	uint32_t mipLevels = 1,
	////	uint32_t arrayLayerCount = 1,
	////	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL
	//	
	//);

	~Image();

	// - Getters
	Device& device() const;
	const VkImage& handle() const;
	const VkImageView& imageView() const;
	const VkExtent3D& extent() const;
	const VkImageSubresource& subresource() const;
	VkFormat format() const;
	VkSampleCountFlagBits sampleCount() const;
	VkImageUsageFlags usage() const;
	VkDeviceMemory memory() const;

	// - Image Management
	////

private:
	Device& mDevice;
	VkImage mHandle{ VK_NULL_HANDLE };

	// - Image attributes
	VkImageType				mType;
	VkExtent3D				mExtent;		// use different constructor if creating a 3D image
	VkImageSubresource		mSubresource;
	VkFormat				mFormat;
	VkImageTiling			mTiling;
	VkSampleCountFlagBits	mSampleCount;
	VkImageUsageFlags		mUsage;
	VkImageLayout			mLayout;
	VkSharingMode			mSharingMode;

	// - Associated with image
	std::unique_ptr<DeviceMemory> mMemory;
	VkImageView mImageView{ VK_NULL_HANDLE };

	// - Image view management
	void createImageView();

	// - Memory management
	void createMemory(VkMemoryPropertyFlags propFlags);

	void createImage();
};

