#include "RenderPass.h"

#include "Device.h"
#include "RenderTarget.h"

RenderPass::RenderPass(Device& device, 
	const std::vector<Attachment>& attachments, 
	const std::vector<SubpassInfo>& subpassInfos,
	const std::vector<LoadStoreInfo>& loadStoreInfos) :
	mDevice(device), mSubpassCount(subpassInfos.size())
{
	assert(attachments.size() == loadStoreInfos.size() && "A load store info must exist for each attachment!");

	// CREATE ATTACHMENT DESCRIPTIONS
	std::vector<VkAttachmentDescription> attachmentDescriptions = {};
	createAttachmentDescriptions(attachments,
		loadStoreInfos,
		attachmentDescriptions);

	// CREATE ATTACHMENT REFERENCES
	// Vector for input attachments - first index refers to subpass, second index refers to the reference of an input attachment
	std::vector<std::vector<VkAttachmentReference>> inputAttachmentReferences(subpassInfos.size());

	// Vector of output attachments - must have a vector for each type of attachment as required in the subpass descrip struct
	std::vector<std::vector<VkAttachmentReference>> colourAttachmentReferences(subpassInfos.size());
	std::vector<std::vector<VkAttachmentReference>> depthStencilAttachmentReferences(subpassInfos.size());
	//std::vector<std::vector<VkAttachmentReference>> resolveAttachmentReferences = {};

	// TODO : need to update to work for resolve attachments
	for (size_t i = 0; i < subpassInfos.size(); ++i)
	{
		auto& subpassInfo = subpassInfos[i];

		// Create input attachment references
		for (uint32_t inputAttachment : subpassInfo.inputAttachments)
		{
			VkImageLayout defaultLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			VkImageLayout initialLayout = attachments[inputAttachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED ? defaultLayout : attachments[inputAttachment].initialLayout;
			inputAttachmentReferences[i].push_back({ inputAttachment, initialLayout });
		}

		// Store input attachment count for this subpass
		mInputAttachmentCounts.push_back(inputAttachmentReferences[i].size());

		// Create output colour/depth attachment references
		for (uint32_t outputAttachment : subpassInfo.outputAttachments)
		{
			bool isDepth = isDepthStencilFormat(attachmentDescriptions[outputAttachment].format);

			VkImageLayout defaultLayout = isDepth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			VkImageLayout initialLayout = attachments[outputAttachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED ? defaultLayout : attachments[outputAttachment].initialLayout;
			
			if (isDepth)
			{
				depthStencilAttachmentReferences[i].push_back({ outputAttachment, initialLayout });
			}
			else
			{
				colourAttachmentReferences[i].push_back({ outputAttachment, initialLayout });
			}
		}

		// Store colour and depth attachment count for this subpass
		mColourAttachmentCounts.push_back(colourAttachmentReferences[i].size());
		mDepthAttachmentCounts.push_back(depthStencilAttachmentReferences[i].size());

	}

	// CREATE SUBPASS DESCRIPTIONS
	std::vector<VkSubpassDescription> subpassDescriptions = {};
	for (size_t i = 0; i < subpassInfos.size(); ++i)
	{
		VkSubpassDescription subpassDescription = {};

		subpassDescription.pipelineBindPoint	= VK_PIPELINE_BIND_POINT_GRAPHICS;

		subpassDescription.inputAttachmentCount = static_cast<uint32_t>(inputAttachmentReferences[i].size());
		subpassDescription.pInputAttachments	= inputAttachmentReferences[i].empty() ? nullptr : inputAttachmentReferences[i].data();

		subpassDescription.colorAttachmentCount = static_cast<uint32_t>(colourAttachmentReferences[i].size());
		subpassDescription.pColorAttachments	= colourAttachmentReferences[i].empty() ? nullptr : colourAttachmentReferences[i].data();

		subpassDescription.pDepthStencilAttachment = depthStencilAttachmentReferences[i].empty() ? nullptr : depthStencilAttachmentReferences[i].data();

		// TODO : add resolve attachments

		subpassDescriptions.push_back(std::move(subpassDescription));
	}

	// CREATE SUBPASS DEPENDENCIES
	std::vector<VkSubpassDependency> subpassDependencies = {};

	if (subpassInfos.size() > 1)	// Dependencies not required with only 1 subpass
	{
		VkSubpassDependency subpassDependency = {};

		// First Subpass must not have any input attachments
		assert(subpassInfos[0].inputAttachments.empty() && "First Subpass must not have any input attachments! Ensure that the subpassInfos vector is ordered correctly.");

		subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;								// EXTERNAL as src refers to commands prior to vkCmdBeginRenderPass
		subpassDependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;			// BOTTOM as src means all stages in commands prior to vkCmdBeginRenderPass
		subpassDependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;					// Specifies all read access types

		subpassDependency.dstSubpass = 1;												// Destination will be current subpass (purpose of this dependency is to show this subpass has no prior execution dependencies)
		subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Stage where colour values are output + subpass load/store operations and resolve operations occur
		subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |			// Specifies read access to a color attachment
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;										// Specifies write access to a colour, resolve or depth/stencil resolve attachment

		// All dependencies should have the below dependency flags
		subpassDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;				// makes dependency framebuffer local

		subpassDependencies.push_back(subpassDependency);

		// Create intermediate dependencies (from one subpass to another)
		for (uint32_t i = 0; i < static_cast<uint32_t>(subpassInfos.size() - 1); ++i)
		{
			subpassDependency.srcSubpass = i;
			subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			subpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;		

			subpassDependency.dstSubpass = i+1;
			subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;	// TOP as dst refers to all stages in commands after vkCmdEndRenderPass
			subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |			// Specifies read access to a color attachment
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;										// Specifies write access to a colour, resolve or depth/stencil resolve attachment
			
			subpassDependencies.push_back(subpassDependency);
		}

		// Add dependency for final subpass with no output attachments
		size_t finalSubpassIndex = subpassInfos.size() - 1;

		// Final subpass must output to swapchain attachment (this should always be attachment 0)
		assert(subpassInfos[finalSubpassIndex].outputAttachments[0] == 0 && "Final Subpass must not have any output attachments! Ensure that the subpassInfos vector is ordered correctly.");

		subpassDependency.srcSubpass = finalSubpassIndex;
		subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		subpassDependency.dstSubpass = VK_SUBPASS_EXTERNAL;							// EXTERNAL as dst refers to commands after vkCmdEndRenderPass
		subpassDependency.dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;			// TOP as dst refers to all stages in commands after vkCmdEndRenderPass
		subpassDependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;				// Specifies all read access types

		subpassDependencies.push_back(subpassDependency);
	}
	
	// Create info for renderpass
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
	renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
	renderPassCreateInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
	renderPassCreateInfo.pSubpasses = subpassDescriptions.data();
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
	renderPassCreateInfo.pDependencies = subpassDependencies.data();

	VkResult result = vkCreateRenderPass(mDevice.logicalDevice(), &renderPassCreateInfo, nullptr, &mHandle);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Render Pass!");
	}

}

RenderPass::~RenderPass()
{
	if (mHandle != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(mDevice.logicalDevice(), mHandle, nullptr);
	}
}

VkRenderPass RenderPass::handle() const
{
	return mHandle;
}

uint32_t RenderPass::subpassCount() const
{
	return mSubpassCount;
}

uint32_t RenderPass::inputAttachmentCount(uint32_t subpassIndex) const
{
	return mInputAttachmentCounts[subpassIndex];
}

uint32_t RenderPass::colourAttachmentCount(uint32_t subpassIndex) const
{
	return mColourAttachmentCounts[subpassIndex];
}

uint32_t RenderPass::depthAttachmentCount(uint32_t subpassIndex) const
{
	return mDepthAttachmentCounts[subpassIndex];
}

void RenderPass::createAttachmentDescriptions(const std::vector<Attachment>& attachments, 
	const std::vector<LoadStoreInfo>& loadStoreInfos,
	std::vector<VkAttachmentDescription>& attachmentDescriptions)
{
	attachmentDescriptions = {};

	for (size_t i = 0; i < attachments.size(); ++i)
	{
		VkAttachmentDescription attachmentDescription = {};

		attachmentDescription.format = attachments[i].format;
		attachmentDescription.samples = attachments[i].sampleCount;
		attachmentDescription.initialLayout = attachments[i].initialLayout;

		// Attachment 0 should always be the swapchain image
		if (i == 0)
		{
			attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}
		else
		{
			attachmentDescription.finalLayout = isDepthStencilFormat(attachments[i].format) ?
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		

		attachmentDescription.loadOp = loadStoreInfos[i].loadOp;
		attachmentDescription.storeOp = loadStoreInfos[i].storeOp;
		attachmentDescription.stencilLoadOp = loadStoreInfos[i].loadOp;
		attachmentDescription.stencilStoreOp = loadStoreInfos[i].storeOp;

		attachmentDescriptions.push_back(std::move(attachmentDescription));
	}

}
