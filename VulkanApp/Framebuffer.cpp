#include "Framebuffer.h"

#include "Device.h"
#include "ImageView.h"
#include "RenderTarget.h"

//Framebuffer::Framebuffer(Device& device, const VkExtent2D& extent, const std::vector<VkImageView>& attachments, VkRenderPass renderPass) :
//	mDevice(mDevice)
//{
//	VkFramebufferCreateInfo framebufferCreateInfo = {};
//	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//	framebufferCreateInfo.renderPass = renderPass;											// Render pass layout the framebuffer will be used with
//	framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
//	framebufferCreateInfo.pAttachments = attachments.data();								// List of attachments (1:1 with Render Pass)
//	framebufferCreateInfo.width = extent.width;												// Framebuffer width
//	framebufferCreateInfo.height = extent.height;											// Framebuffer height
//	framebufferCreateInfo.layers = 1;														// Framebuffer layers
//
//	VkResult result = vkCreateFramebuffer(device.logicalDevice(), &framebufferCreateInfo, nullptr, &mHandle);
//	if (result != VK_SUCCESS)
//	{
//		throw std::runtime_error("Failed to create a Framebuffer");
//	}
//}

Framebuffer::Framebuffer(Device& device, VkRenderPass renderPass, const RenderTarget& renderTarget) :
	mDevice(device)
{
	auto& extent = renderTarget.extent();
	auto& imageViews = renderTarget.imageViews();
	std::vector<VkImageView> framebufferAttachments(imageViews.size(), VK_NULL_HANDLE);

	std::transform(imageViews.begin(), imageViews.end(), framebufferAttachments.begin(),
		[](const ImageView& object) { return object.handle(); });

	VkFramebufferCreateInfo framebufferCreateInfo = {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.renderPass = renderPass;											// Render pass layout the framebuffer will be used with
	framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(framebufferAttachments.size());
	framebufferCreateInfo.pAttachments = framebufferAttachments.data();								// List of attachments (1:1 with Render Pass)
	framebufferCreateInfo.width = extent.width;												// Framebuffer width
	framebufferCreateInfo.height = extent.height;											// Framebuffer height
	framebufferCreateInfo.layers = 1;														// Framebuffer layers

	VkResult result = vkCreateFramebuffer(device.logicalDevice(), &framebufferCreateInfo, nullptr, &mHandle);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Framebuffer");
	}
}

Framebuffer::~Framebuffer()
{
	if (mHandle != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(mDevice.logicalDevice(), mHandle, nullptr);
	}
	
}

VkFramebuffer Framebuffer::handle() const
{
	return mHandle;
}
