#pragma once
#include "Common.h"

class Device;
class Image;

struct Attachment {
	VkFormat format;

	VkSampleCountFlagBits sampleCount;

	VkImageUsageFlags usage;

	VkImageLayout initialLayout{ VK_IMAGE_LAYOUT_UNDEFINED };

	Attachment() = default;
	Attachment(VkFormat format, VkSampleCountFlagBits sampleCount, VkImageUsageFlags usage);
};

// Contains all images handled in the renderpass and their associated imageviews
// attachment indices are used to indicate which attachments are used in the input and output of a subpass
class RenderTarget
{
public:
	RenderTarget(std::vector<Image>& images);
	~RenderTarget();

	// - Getters
	const VkExtent2D& extent() const;
	const std::vector<VkImageView>& imageViews() const;
	const std::vector<Attachment>& attachments() const;

	const std::vector<uint32_t>& inputAttachmentIndices() const;
	const std::vector<uint32_t>& outputAttachmentsIndices() const;

	// - Setters
	void setLayout(uint32_t attachmentIndex, VkImageLayout layout);

	// OBSOLETE?
	void setInputAttachmentIndices(std::vector<uint32_t> newIndices);
	void setOutputAttachmentIndices(std::vector<uint32_t> newIndices);
private:
	Device& mDevice;

	VkExtent2D mExtent;

	std::vector<Image> mImages;
	std::vector<VkImageView> mImageViews;

	std::vector<Attachment> mAttachments;

	std::vector<uint32_t> mInputAttachmentIndices;

	std::vector<uint32_t> mOutputAttachmentIndices;

};

