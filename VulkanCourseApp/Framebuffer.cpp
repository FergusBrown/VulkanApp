#include "Framebuffer.h"

Framebuffer::Framebuffer(Device& device, const VkExtent2D& extent, const std::vector<VkImageView>& attachments, VkRenderPass renderPass) :
	mDevice(mDevice)
{
	VkFramebufferCreateInfo framebufferCreateInfo = {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.renderPass = renderPass;											// Render pass layout the framebuffer will be used with
	framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferCreateInfo.pAttachments = attachments.data();								// List of attachments (1:1 with Render Pass)
	framebufferCreateInfo.width = extent.width;												// Framebuffer width
	framebufferCreateInfo.height = extent.height;											// Framebuffer height
	framebufferCreateInfo.layers = 1;														// Framebuffer layers

	VkResult result = vkCreateFramebuffer(mDevice.logicalDevice(), &framebufferCreateInfo, nullptr, &mFramebuffer);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Framebuffer");
	}
}

Framebuffer::~Framebuffer()
{
	if (mFramebuffer != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(mDevice.logicalDevice(), mFramebuffer, nullptr);
	}
	
}
