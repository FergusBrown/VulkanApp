#pragma once
#include "Renderer/VulkanRenderer.h"

class DeferredApp : public VulkanRenderer
{
public:
	DeferredApp() = default;
	~DeferredApp();

	virtual void draw();
private:
	// Variables
	uint32_t mVPBufferIndex{ 0 };
	uint32_t mLightBufferIndex{ 0 };

	// RENDERPASS 0
	// Subpass attachment indices
	uint32_t mLightingAttachmentIndex{ 0 };
	uint32_t mPositionAttachmentIndex{ 0 };
	uint32_t mNormalAttachmentIndex{ 0 };
	uint32_t mAlbedoSpecAttachmentIndex{ 0 };
	uint32_t mDepthAttachmentIndex{ 0 };

	// Additional formats
	VkFormat mPositionFormat{};

	float lastTime{ 0.0f };
	float sumTime{ 0.0f };

	// Buffer compositions
	struct uboLights {
		PointLight pointLights[3];
		SpotLight flashLight;	// Note that flashlight contains view position which will be used for lighting calculations
	} mLights;

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
	virtual void getRequiredExtenstionAndFeatures(std::vector<const char*> & requiredExtensions,
		VkPhysicalDeviceFeatures & requiredFeatures);

	void createLights();

	// - Record Functions
	void recordCommands(CommandBuffer & primaryCmdBuffer);
	CommandBuffer* recordSecondaryCommandBuffers(CommandBuffer * primaryCommandBuffer,
		std::vector<std::reference_wrapper<Mesh>> meshList,
		uint32_t meshStart,
		uint32_t meshEnd,
		size_t threadIndex);
};

