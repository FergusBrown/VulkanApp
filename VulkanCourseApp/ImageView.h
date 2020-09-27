#pragma once
#include "Common.h"

class Device;
class Image;

class ImageView
{
public:
	ImageView(Image &image,
		VkImageViewType viewtype,
		VkFormat format = VK_FORMAT_UNDEFINED);
	~ImageView();

	ImageView(const ImageView&) = delete;

	ImageView(ImageView&& other);

	// - Getters
	VkImageView handle() const;
	const Image& image() const;

private:
	Device& mDevice;

	Image* mImage{ nullptr };

	VkImageView mHandle{ VK_NULL_HANDLE };

	VkFormat mFormat{};

	VkImageSubresourceRange mSubresourceRange{};

};

