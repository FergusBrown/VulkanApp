#pragma once
#include "Common.h"
#include "Device.h"

#include "Image.h"

struct Attachment {
	VkFormat format;

	VkSampleCountFlagBits sampleCount;

	VkImageUsageFlags usage;

	VkImageLayout initialLayout;

	Attachment() = default;
	Attachment(VkFormat format, VkSampleCountFlagBits sampleCount, VkImageUsageFlags usage);
};

class RenderTarget
{
public:
	RenderTarget(std::vector<Image>& images);
	~RenderTarget();

	// - Getters
	const VkExtent3D& extent() const;
	const std::vector<VkImageView>& imageViews() const;
	const std::vector<Attachment>& attachments() const;

	const std::vector<uint32_t>& inputAttachmentIndices() const;
	const std::vector<uint32_t>& outputAttachmentsIndices() const;

	// - Setters
	void setLayout(uint32_t attachmentIndex, VkImageLayout layout);
	void setInputAttachmentIndices(std::vector<uint32_t> newIndices);
	void setOutputAttachmentIndices(std::vector<uint32_t> newIndices);
private:
	Device& mDevice;

	VkExtent3D mExtent;

	std::vector<Image>& mImages;
	std::vector<VkImageView> mImageViews;

	std::vector<Attachment> mAttachments;

	std::vector<uint32_t> mInputAttachmentIndices;

	std::vector<uint32_t> mOutputAttachmentIndices;

};

