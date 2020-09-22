#pragma once
#include "Common.h"	

class Device;
//class RenderTarget;

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

