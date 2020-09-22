#include "DescriptorSetLayout.h"

#include "Device.h"

DescriptorSetLayout::DescriptorSetLayout(Device& device, uint32_t setIndex, std::vector<ShaderResource>& shaderResources) :
	mDevice(device), mSetIndex(setIndex)
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	// Create bindings
	for (auto& resource : shaderResources)
	{
		createDescriptorSetLayoutBinding(bindings, resource);
	}

	createDescriptorSetLayout(bindings);
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	if (mHandle != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(mDevice.logicalDevice(), mHandle, nullptr);
	}
}

Device& DescriptorSetLayout::device()
{
	return mDevice;
}

VkDescriptorSetLayout DescriptorSetLayout::handle() const
{
	return mHandle;
}

const std::vector<VkDescriptorSetLayoutBinding>& DescriptorSetLayout::layoutBindings() const
{
	return mLayoutBindings;
}

const VkDescriptorSetLayoutBinding& DescriptorSetLayout::layoutBinding(uint32_t bindingIndex) const
{
	return mLayoutBindings.at(bindingIndex);
}

void DescriptorSetLayout::createDescriptorSetLayoutBinding(std::vector<VkDescriptorSetLayoutBinding>& bindings, ShaderResource& shaderResource)
{
	VkDescriptorSetLayoutBinding layoutBinding = {};
	layoutBinding.binding = shaderResource.binding;						// Binding point in shader ( designated by binding number in shader)
	layoutBinding.descriptorType = shaderResource.descriptorType;		// Type of descriptor (uniform, dynamic uniform, image sampler, etc)
	layoutBinding.descriptorCount = shaderResource.descriptorCount;		// Numbers of descriptors for binding
	layoutBinding.stageFlags = shaderResource.stageFlags;				// Shader stage to bind to
	layoutBinding.pImmutableSamplers = nullptr;

	bindings.push_back(layoutBinding);
}

void DescriptorSetLayout::createDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());					// Number of binding infos
	layoutCreateInfo.pBindings = bindings.data();											// Array of binding infos

	// Create descriptor set layout
	VkResult result = vkCreateDescriptorSetLayout(mDevice.logicalDevice(), &layoutCreateInfo, nullptr, &mHandle);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Descriptor Set Layout!");
	}
}

ShaderResource::ShaderResource(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags) :
	binding(binding), descriptorType(descriptorType), descriptorCount(descriptorCount), stageFlags(stageFlags)
{
}
