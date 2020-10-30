#pragma once
#include "Renderer/VulkanRenderer.h"

class SSAOApp : public VulkanRenderer
{
public:
	SSAOApp() = default;
	~SSAOApp();

	virtual void draw();

private:
	// SSAO Resources
	// SUBPASS 1
	std::unique_ptr<Texture> mNoiseTexture;
	std::unique_ptr<Sampler> mPositionSampler;
	std::unique_ptr<Sampler> mNormalSampler;
	std::unique_ptr<Sampler> mNoiseSampler;

	// SUBPASS 2
	std::unique_ptr<Sampler> mSSAOSampler;

	// UBO indices
	uint32_t mVPBufferIndex{ 0 };
	uint32_t mLightBufferIndex{ 0 };
	uint32_t mSSAOBufferIndex{ 0 };

	// ATTACHMENT INDICES
	// SUBPASS 2
	uint32_t mBlurAttachmentIndex{ 0 };
	// SUBPASS 1
	uint32_t mSSAOAttachmentIndex{ 0 };
	// SUBPASS 0
	uint32_t mPositionAttachmentIndex{ 0 };
	uint32_t mNormalAttachmentIndex{ 0 };
	uint32_t mAlbedoAttachmentIndex{ 0 };
	uint32_t mSpecularAttachmentIndex{ 0 };
	uint32_t mDepthAttachmentIndex{ 0 };

	// Additional formats
	VkFormat mPrecisionFormat{};

	// Time parameters
	float lastTime{ 0.0f };
	float sumTime{ 0.0f };

	// Buffer compositions
	struct uboLights {
		PointLight pointLights[3];
		SpotLight flashLight;	// Note that flashlight contains view position which will be used for lighting calculations
	} mLights;

#define SAMPLE_COUNT 64
	struct uboSSAO {
		glm::vec3 ssaoKernel[SAMPLE_COUNT];
	};

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

	// Shader resource creation
	void createLights();
	void createSSAOResources();
	void createAttachmentSamplers();

	// - Record Functions
	void recordCommands(CommandBuffer& primaryCmdBuffer);
	CommandBuffer* recordSecondaryCommandBuffers(CommandBuffer* primaryCommandBuffer,
		std::vector<std::reference_wrapper<Mesh>> meshList,
		uint32_t meshStart,
		uint32_t meshEnd,
		size_t threadIndex);
};

