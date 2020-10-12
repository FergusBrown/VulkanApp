#pragma once
#include "Renderer/VulkanRenderer.h"

// A simple application using:
// - Depth buffer
// - Multiple subpasses
// - Multithreaded command buffer submission
class DepthBufferApp : public VulkanRenderer
{
public:
	DepthBufferApp() = default;
	~DepthBufferApp() = default;

	virtual void draw();
private:

	// Vulkan Components
	// - Main

	// - Formats
	VkFormat mColourFormat{ VK_FORMAT_R8G8B8A8_UNORM };
	VkFormat mDepthFormat{ VK_FORMAT_D32_SFLOAT_S8_UINT };

	// - Descriptors
	std::unique_ptr<DescriptorSetLayout> mAttachmentSetLayout;
	VkPushConstantRange mPushConstantRange;

	std::vector<std::unique_ptr<DescriptorResourceReference>> mAttachmentResources;

	std::unique_ptr<DescriptorPool> mAttachmentDescriptorPool;

	std::vector<std::unique_ptr<DescriptorSet>> mAttachmentDescriptorSets;		// Descriptor set holding colour/depth images (used for second subpass)

	// Vulkan Functions
	// - Create Functions
	virtual void createRenderTargetAndFrames();
	virtual void createRenderPass();
	virtual void createDescriptorSetLayouts();
	virtual void createPushConstantRange();
	virtual void createPipelines();
	virtual void createDescriptorPools();
	virtual void createDescriptorSets();

	// - Record Functions
	void recordCommands(CommandBuffer& primaryCmdBuffer);
	CommandBuffer* recordSecondaryCommandBuffers(CommandBuffer* primaryCommandBuffer, uint32_t objectStart, uint32_t objectEnd, size_t threadIndex);
};

