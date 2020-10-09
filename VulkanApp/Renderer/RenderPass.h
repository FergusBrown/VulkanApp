#pragma once
#include "Common.h"

struct Attachment;
class Device;

// Info for creating a subpass
struct SubpassInfo
{
	std::vector<uint32_t> inputAttachments;

	std::vector<uint32_t> outputAttachments;
};

// Load store info for a render pass attachment
struct LoadStoreInfo
{
	VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

	VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE;
};

// Object to simplify Render Pass creation and contain Render Pass handle + associated data
class RenderPass
{
public:
	RenderPass(Device& device,
		const std::vector<Attachment>& attachments,
		const std::vector<SubpassInfo>& subpassInfos,
		const std::vector<LoadStoreInfo>& loadStoreInfos);

	~RenderPass();

	// - Getters
	VkRenderPass handle() const;

	uint32_t subpassCount() const;

private:
	Device& mDevice;

	VkRenderPass mHandle{ VK_NULL_HANDLE };

	uint32_t mSubpassCount{ 1 };

	// - Support
	void createAttachmentDescriptions(const std::vector<Attachment>& attachments, 
		const std::vector<LoadStoreInfo>& loadStoreInfos, 
		std::vector<VkAttachmentDescription>& attachmentDescriptions);

	//void createAttachmentReferences

};

