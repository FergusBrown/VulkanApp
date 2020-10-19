#pragma once
#include "Renderer/VulkanRenderer.h"

// A simple application using:
// - Basic Shading
// - Depth buffer
// - Multiple subpasses
// - Multithreaded command buffer submission
class BasicApp : public VulkanRenderer
{
public:
	BasicApp() = default;
	~BasicApp();

	virtual void draw();
private:
	// Variables
	uint32_t mUniformBufferIndex{ 0 };

	uint32_t mColourAttachmentIndex{ 0 };
	uint32_t mDepthAttachmentIndex{ 0 };

	// Functions
	// - Create Functions
	virtual void createRenderTargetAndFrames();
	virtual void createRenderPass();
	virtual void createPerFrameDescriptorSetLayouts();
	virtual void createPipelines();
	virtual void createPerFrameResources();
	virtual void createPerFrameDescriptorSets();

	// -- Support
	virtual void updatePerFrameResources();
	virtual void getRequiredExtenstionAndFeatures(std::vector<const char*>& requiredExtensions,
		VkPhysicalDeviceFeatures& requiredFeatures);

	// - Record Functions
	void recordCommands(CommandBuffer& primaryCmdBuffer);
	CommandBuffer* recordSecondaryCommandBuffers(CommandBuffer* primaryCommandBuffer, 
		std::vector<std::reference_wrapper<Mesh>> meshList,
		uint32_t meshStart, 
		uint32_t meshEnd, 
		size_t threadIndex);
};
