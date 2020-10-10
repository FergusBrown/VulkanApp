#include "PipelineLayout.h"

#include "Device.h"
#include "DescriptorSetLayout.h"

PipelineLayout::PipelineLayout(Device& device,
	const std::vector<std::reference_wrapper<DescriptorSetLayout>>& descriptorSetLayouts,
	const VkPushConstantRange& pushConstantRange) :
	mDevice(device)
{
	std::vector<VkDescriptorSetLayout> layoutHandles(descriptorSetLayouts.size());

	std::transform(descriptorSetLayouts.begin(), descriptorSetLayouts.end(), layoutHandles.begin(),
		[](const DescriptorSetLayout& descriptorSetLayout) { return descriptorSetLayout.handle(); });

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(layoutHandles.size());
	pipelineLayoutCreateInfo.pSetLayouts = layoutHandles.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

	//Create Pipeline Layout
	VkResult result = vkCreatePipelineLayout(mDevice.logicalDevice(), &pipelineLayoutCreateInfo, nullptr, &mHandle);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Pipeline Layout!");
	}
}

PipelineLayout::PipelineLayout(Device& device, const std::vector<std::reference_wrapper<DescriptorSetLayout>>& descriptorSetLayouts) :
	mDevice(device)
{
	std::vector<VkDescriptorSetLayout> layoutHandles(descriptorSetLayouts.size());

	std::transform(descriptorSetLayouts.begin(), descriptorSetLayouts.end(), layoutHandles.begin(),
		[](const DescriptorSetLayout& descriptorSetLayout) { return descriptorSetLayout.handle(); });

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(layoutHandles.size());
	pipelineLayoutCreateInfo.pSetLayouts = layoutHandles.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	//Create Pipeline Layout
	VkResult result = vkCreatePipelineLayout(mDevice.logicalDevice(), &pipelineLayoutCreateInfo, nullptr, &mHandle);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Pipeline Layout!");
	}
}

PipelineLayout::~PipelineLayout()
{
	if (mHandle != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(mDevice.logicalDevice(), mHandle, nullptr);
	}
}

VkPipelineLayout PipelineLayout::handle() const
{
	return mHandle;
}
