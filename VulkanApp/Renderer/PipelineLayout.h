#pragma once
#include "Common.h"

class Device;
class DescriptorSetLayout;

class PipelineLayout
{
public:
	PipelineLayout(Device& device,
		const std::vector<std::reference_wrapper<const DescriptorSetLayout>>& descriptorSetLayouts,
		const VkPushConstantRange& pushConstantRange);

	PipelineLayout(Device& device,
		const std::vector<std::reference_wrapper<const DescriptorSetLayout>>& descriptorSetLayouts);
	~PipelineLayout();

	PipelineLayout(const PipelineLayout&) = delete;

	VkPipelineLayout handle() const;

private:

	Device& mDevice;

	VkPipelineLayout mHandle{ VK_NULL_HANDLE };
};

