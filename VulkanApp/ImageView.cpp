#include "ImageView.h"

#include "Device.h"
#include "Image.h"

ImageView::ImageView(Image& image, VkImageViewType viewtype, VkFormat format) :
    mDevice(image.device()), mImage(&image), mFormat(format)
{
	if (format == VK_FORMAT_UNDEFINED)
	{
		mFormat = image.format();
	}

	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = image.handle();								// Image to create view for
	viewCreateInfo.viewType = viewtype;									// Type of image (1D, 2D, 3D etc.)
	viewCreateInfo.format = mFormat;										// Format of image data
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;		// Allows remapping of rgba components to other rgba values
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	auto& subresource = image.subresource();

	// Which aspect of image to view (e.g. COLOR_BIT for viewing color)
	if (isDepthStencilFormat(mFormat))
	{
		mSubresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else {
		mSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	
							
	mSubresourceRange.baseArrayLayer = 0;					// Start mipmap level to view from
	mSubresourceRange.layerCount = subresource.arrayLayer;	// Number of mipmap levels to view
	mSubresourceRange.baseMipLevel = 0;						// Starrt array level to view from
	mSubresourceRange.levelCount = subresource.mipLevel;	// Number of array levels to view


	// Subresource range is the range of subresources which the view can access
	viewCreateInfo.subresourceRange = mSubresourceRange;		

	// Create Image View
	VkResult result = vkCreateImageView(mDevice.logicalDevice(), &viewCreateInfo, nullptr, &mHandle);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create an Image View!");
	}

}

ImageView::~ImageView()
{
    if (mHandle != VK_NULL_HANDLE)
    {
        vkDestroyImageView(mDevice.logicalDevice(), mHandle, nullptr);
    }
}

//ImageView::ImageView(ImageView&& other)
//{
//}

ImageView::ImageView(ImageView&& other) :
	mDevice(other.mDevice),
	mImage(other.mImage),
	mFormat(other.mFormat),
	mHandle(other.mHandle),
	mSubresourceRange(other.mSubresourceRange)
{
	other.mHandle = VK_NULL_HANDLE;
}

VkImageView ImageView::handle() const
{
    return mHandle;
}

const Image& ImageView::image() const
{
    assert(mImage && "No Image bound is to this Image View!");

	return *mImage;
}
