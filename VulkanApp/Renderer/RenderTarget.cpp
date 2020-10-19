#include "RenderTarget.h"

#include "Device.h"
#include "Image.h"
#include "ImageView.h"

Attachment::Attachment(VkFormat format, VkSampleCountFlagBits sampleCount, VkImageUsageFlags usage) :
	format(format), sampleCount(sampleCount), usage(usage)
{
}

RenderTarget::RenderTarget(std::vector<Image>&& images) :
	mImages(std::move(images)), mDevice(images.back().device()), mExtent({ images.back().extent().width, images.back().extent().height })
{
	for (auto& image : mImages)
	{
		mImageViews.emplace_back(image, VK_IMAGE_VIEW_TYPE_2D);

		mAttachments.emplace_back(image.format(), image.sampleCount(), image.usage());
	}
}

RenderTarget::~RenderTarget()
{

}

const VkExtent2D& RenderTarget::extent() const
{
	return mExtent;
}


const std::vector<ImageView>& RenderTarget::imageViews() const 
{
	return mImageViews;
}

const std::vector<Attachment>& RenderTarget::attachments() const
{
	return mAttachments;
}

const std::vector<uint32_t>& RenderTarget::inputAttachments() const
{
	return mInputAttachments;
}

const std::vector<uint32_t>& RenderTarget::outputAttachments() const
{
	return mOutputAttachments;
}

void RenderTarget::setLayout(uint32_t attachmentIndex, VkImageLayout layout)
{
	mAttachments[attachmentIndex].initialLayout = layout;
}

void RenderTarget::setInputAttachments(const std::vector<uint32_t>& inputAttachments)
{
	mInputAttachments = inputAttachments;
}

void RenderTarget::setOutputAttachments(const std::vector<uint32_t>& outputAttachments)
{
	mOutputAttachments = outputAttachments;
}
