#pragma once
#include "Common.h"

class Device;
class PipelineLayout;
class RenderPass;
class ShaderModule;
class Swapchain;

class Pipeline
{
public:
	Pipeline(Device& device);
	virtual ~Pipeline();

	// - Getters
	VkPipeline handle() const;

protected:
	Device& mDevice;

	VkPipeline mHandle{VK_NULL_HANDLE};

};

class GraphicsPipeline : public Pipeline
{
public:
	GraphicsPipeline(Device& device,
		const std::vector<ShaderModule>& shaderModules,
		const Swapchain& swapchain,
		const PipelineLayout& pipelineLayout,
		const RenderPass& renderPass,
		uint32_t subpassIndex,
		VkBool32 vertexInput,
		VkBool32 depthWriteEnable);

	virtual ~GraphicsPipeline() = default;

};