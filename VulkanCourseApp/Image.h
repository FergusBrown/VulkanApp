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
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL
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
	VkImage mImage{ VK_NULL_HANDLE };

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
	VkDeviceMemory mMemory{ VK_NULL_HANDLE };
	VkImageView mImageView{ VK_NULL_HANDLE };

	// - Image view management
	void createImageView();

	// - Memory management
	void createMemory();
};

