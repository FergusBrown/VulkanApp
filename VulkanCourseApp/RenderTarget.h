#pragma once
#include "Common.h"
#include "Device.h"

struct Attachment {
	VkFormat format;

	VkSampleCountFlagBits samples;

	VkImageUsageFlags usage;

	VkImageLayout initial_layout;

	Attachment() = default;

	Attachment(VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage);
};

class RenderTarget
{
public:
	RenderTarget(Device& device, VkExtent2D extent, std::vector<VkImage>& images);
	~RenderTarget();

	const std::vector<VkImageView>& imageViews() const;
	const std::vector<Attachment>& attachments() const;


private:
	Device& mDevice;

	VkExtent2D mExtent;

	std::vector<VkImage> mImages;

	std::vector<VkImageView> mImageViews;

	std::vector<Attachment> mAttachments;

	std::vector<uint32_t> inputAttachments;

	std::vector<uint32_t> outputAttachments;

};

