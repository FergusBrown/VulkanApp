#include "DescriptorSet.h"

DescriptorSet::DescriptorSet(DescriptorPool& descriptorPool) :
	mDescriptorPool(descriptorPool), mDevice(descriptorPool.device())
{
	VkDescriptorSetLayout layoutHandle = descriptorPool.descriptorSetLayout().handle();

	// Descriptor Set Allocation Info
	VkDescriptorSetAllocateInfo setAllocInfo = {};
	setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocInfo.descriptorPool = descriptorPool.handle();					// Pool to allocate the Descriptor Set from
	setAllocInfo.descriptorSetCount = 1;									// Number of sets to allocate
	setAllocInfo.pSetLayouts = &layoutHandle;

	// Allocate descriptor sets 
	VkResult result = vkAllocateDescriptorSets(mDevice.logicalDevice(), &setAllocInfo, &mHandle);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate Descriptor Set!");
	}
}

DescriptorSet::DescriptorSet(Device& device, 
	DescriptorPool& descriptorPool, 
	std::unordered_map<uint32_t, 
	VkDescriptorImageInfo>& imageInfos, 
	std::unordered_map<uint32_t, VkDescriptorBufferInfo>& bufferInfos)
{
	
}

Device& DescriptorSet::device() const
{
	return mDevice;
}

VkDescriptorSet DescriptorSet::handle() const
{
	return mHandle;
}

// If passed vector is empty then update all bindings, otherwise only update selected bindings
void DescriptorSet::update(const std::vector<uint32_t>& bindingsToUpdate)
{
	// TODO
}
