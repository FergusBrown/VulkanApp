#include "RenderTarget.h"

Attachment::Attachment(VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage) :
	format(format), samples(samples), usage(usage)
{
}

RenderTarget::RenderTarget(Device& device, VkExtent2D extent, std::vector<VkImage>& images) :
	mDevice(device), mExtent(extent), mImages(images)
{

}

RenderTarget::~RenderTarget()
{
	for (auto imageView : mImageViews)
	{
		vkDestroyImageView(mDevice.logicalDevice(), imageView, nullptr);
	}
}

const std::vector<VkImageView>& RenderTarget::imageViews() const
{
	return mImageViews;
}

const std::vector<Attachment>& RenderTarget::attachments() const
{
	return mAttachments;
}
