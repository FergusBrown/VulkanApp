#include "DescriptorSet.h"

DescriptorSet::DescriptorSet(Device& device,
	DescriptorSetLayout& descriptorSetLayout,
	DescriptorPool& descriptorPool,
	const BindingMap<VkDescriptorImageInfo>& imageInfos,
	const BindingMap<VkDescriptorBufferInfo>& bufferInfos) :
	mDevice(device),
	mDescriptorPool(descriptorPool),
	mImageInfos(imageInfos),
	mBufferInfos(bufferInfos),
	mDescriptorSetLayout(descriptorSetLayout),
	mHandle(descriptorPool.allocate())
{
	prepareImageWriteOperations();
	prepareBufferWriteOperations();
}

Device& DescriptorSet::device() const
{
	return mDevice;
}

VkDescriptorSet DescriptorSet::handle() const
{
	return mHandle;
}

const BindingMap<VkDescriptorImageInfo>& DescriptorSet::imageInfos() const
{
	return mImageInfos;
}

const BindingMap<VkDescriptorBufferInfo>& DescriptorSet::bufferInfos() const
{
	return mBufferInfos;
}

// If passed vector is empty then update all bindings, otherwise only update selected bindings
void DescriptorSet::update(const std::vector<uint32_t>& bindingsToUpdate)
{
	// TODO
}

void DescriptorSet::prepareImageWriteOperations()
{
	for (auto& binding : mImageInfos)
	{
		uint32_t bindingIndex = binding.first;
		auto& bindingContents = binding.second;

		VkDescriptorType descriptorType = mDescriptorSetLayout.layoutBinding(bindingIndex).descriptorType;

		for (auto& descriptor : bindingContents)
		{
			uint32_t descriptorIndex = descriptor.first;
			auto& descriptorInfo = descriptor.second;

			VkWriteDescriptorSet setWrite = {};
			setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			setWrite.dstSet = mHandle;											// Desctriptor set to update
			setWrite.dstBinding = bindingIndex;									// Binding to update (matches with binding on layout/shader)
			setWrite.dstArrayElement = descriptorIndex;							// Index in array to update
			setWrite.descriptorType = descriptorType;		// Type of descriptor
			setWrite.descriptorCount = 1;										// amount to update
			setWrite.pImageInfo = &descriptorInfo;								// Information about image data to bind

			mWriteOperations.push_back(setWrite);
		}
	}
	

}

void DescriptorSet::prepareBufferWriteOperations()
{
	for (auto& binding : mBufferInfos)
	{
		uint32_t bindingIndex = binding.first;
		auto& bindingContents = binding.second;

		VkDescriptorType descriptorType = mDescriptorSetLayout.layoutBinding(bindingIndex).descriptorType;

		for (auto& descriptor : bindingContents)
		{
			uint32_t descriptorIndex = descriptor.first;
			auto& descriptorInfo = descriptor.second;

			VkWriteDescriptorSet setWrite = {};
			setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			setWrite.dstSet = mHandle;											// Desctriptor set to update
			setWrite.dstBinding = bindingIndex;									// Binding to update (matches with binding on layout/shader)
			setWrite.dstArrayElement = descriptorIndex;							// Index in array to update
			setWrite.descriptorType = descriptorType;							// Type of descriptor
			setWrite.descriptorCount = 1;										// amount to update
			setWrite.pBufferInfo = &descriptorInfo;								// Information about buffer data to bind

			mWriteOperations.push_back(setWrite);
		}
	}
}
