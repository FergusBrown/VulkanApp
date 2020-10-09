#pragma once
#include "Common.h"

class Device;
class Image;
class ImageView;

// Parameters associated with an attachment
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
	RenderTarget(std::vector<Image>&& images);
	~RenderTarget();

	// - Getters
	const VkExtent2D& extent() const;
	const std::vector<ImageView>& imageViews() const; // Note that only image views are exposed
	const std::vector<Attachment>& attachments() const;

	const std::vector<uint32_t>& inputAttachments() const;
	const std::vector<uint32_t>& outputAttachments() const;

	// - Setters
	void setLayout(uint32_t attachmentIndex, VkImageLayout layout);

	// Set target attachments for current subpass
	void setInputAttachments(const std::vector<uint32_t>& inputAttachments = {});
	void setOutputAttachments(const std::vector<uint32_t>& outputAttachments = {});
private:
	Device& mDevice;

	VkExtent2D mExtent;

	std::vector<Image> mImages;
	std::vector<ImageView> mImageViews;

	std::vector<Attachment> mAttachments;

	std::vector<uint32_t> mInputAttachments{};

	std::vector<uint32_t> mOutputAttachments{ 0 };
};

