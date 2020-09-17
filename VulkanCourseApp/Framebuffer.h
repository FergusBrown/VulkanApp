#pragma once
#include "Common.h"	

#include "Device.h"
#include "RenderTarget.h"

class Framebuffer
{
public:
	Framebuffer(Device& device, const VkExtent2D& extent, const std::vector<VkImageView>& imageViews, VkRenderPass renderPass);
	~Framebuffer();

	// - Getters
	VkFramebuffer handle() const;

private:
	Device& mDevice;

	VkFramebuffer mHandle;

};

