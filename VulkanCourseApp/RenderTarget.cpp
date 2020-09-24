#include "RenderTarget.h"

#include "Device.h"
#include "Image.h"

Attachment::Attachment(VkFormat format, VkSampleCountFlagBits sampleCount, VkImageUsageFlags usage) :
	format(format), sampleCount(sampleCount), usage(usage)
{
}

RenderTarget::RenderTarget(std::vector<Image>&& images) :
	mImages(std::move(images)), mDevice(images.back().device()), mExtent({ images.back().extent().width, images.back().extent().height })
{
	for (auto& image : mImages)
	{
		mImageViews.push_back(image.imageView());

		mAttachments.push_back(Attachment(image.format(), image.sampleCount(), image.usage()));
	}
}

RenderTarget::~RenderTarget()
{

}

const VkExtent2D& RenderTarget::extent() const
{
	return mExtent;
}

const std::vector<VkImageView>& RenderTarget::imageViews() const
{
	return mImageViews;
}

const std::vector<Attachment>& RenderTarget::attachments() const
{
	return mAttachments;
}

const std::vector<uint32_t>& RenderTarget::inputAttachmentIndices() const
{
	return mInputAttachmentIndices;
}

const std::vector<uint32_t>& RenderTarget::outputAttachmentsIndices() const
{
	return mOutputAttachmentIndices;
}

void RenderTarget::setLayout(uint32_t attachmentIndex, VkImageLayout layout)
{
	mAttachments[attachmentIndex].initialLayout = layout;
}

void RenderTarget::setInputAttachmentIndices(std::vector<uint32_t> inputAttachmentIndices)
{
	mInputAttachmentIndices = inputAttachmentIndices;
}

void RenderTarget::setOutputAttachmentIndices(std::vector<uint32_t> outputAttachmentIndices)
{
	mOutputAttachmentIndices = outputAttachmentIndices;
}
