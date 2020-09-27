#pragma once
#include "Common.h"	

class Device;
class RenderTarget;
//class RenderTarget;

// Object used to create and contain a framebuffer which maps the renderpass output to rendertarget images
class Framebuffer
{
public:
	//Framebuffer(Device& device, const VkExtent2D& extent, const std::vector<VkImageView>& imageViews, VkRenderPass renderPass);
	Framebuffer(Device& device, VkRenderPass renderPass, const RenderTarget& renderTarget);
	~Framebuffer();

	// - Getters
	VkFramebuffer handle() const;

private:
	Device& mDevice;

	VkFramebuffer mHandle;

};

