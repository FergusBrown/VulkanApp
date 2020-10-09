#pragma once
#include "Common.h"

class Device;

//enum class ShaderResourceType
//{
//	ImageSampler,
//	UniformBuffer,
//	InputAttachment
//};

struct ShaderResource {
	uint32_t				binding;
	VkDescriptorType		descriptorType;
	uint32_t				descriptorCount;
	VkShaderStageFlags		stageFlags;

	ShaderResource() = default;
	ShaderResource(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags);
};

class DescriptorSetLayout
{
public:
	DescriptorSetLayout(Device& device, uint32_t setIndex, std::vector<ShaderResource>& shaderResources);
	~DescriptorSetLayout();

	// - Getters
	Device& device();
	VkDescriptorSetLayout handle() const;

	const std::vector<VkDescriptorSetLayoutBinding>& layoutBindings() const;
	const VkDescriptorSetLayoutBinding& layoutBinding(uint32_t bindingIndex) const;

private:
	Device& mDevice;

	VkDescriptorSetLayout mHandle{ VK_NULL_HANDLE };

	const uint32_t mSetIndex;

	std::vector<VkDescriptorSetLayoutBinding> mLayoutBindings;

	// - Support
	void createDescriptorSetLayoutBinding(ShaderResource& shaderResource);
	void createDescriptorSetLayout();


};

