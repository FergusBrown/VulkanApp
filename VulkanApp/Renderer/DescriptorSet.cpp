#include "DescriptorSet.h"

#include "DescriptorPool.h"
#include "DescriptorSetLayout.h"
#include "Device.h"

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
	prepareWriteOperations();
}

DescriptorSet::DescriptorSet(Device& device,
	DescriptorSetLayout& descriptorSetLayout,
	DescriptorPool& descriptorPool,
	const BindingMap<VkDescriptorBufferInfo>& bufferInfos,
	const BindingMap<VkDescriptorImageInfo>& imageInfos) :
	mDevice(device),
	mDescriptorPool(descriptorPool),
	mImageInfos(imageInfos),
	mBufferInfos(bufferInfos),
	mDescriptorSetLayout(descriptorSetLayout),
	mHandle(descriptorPool.allocate())
{
	prepareWriteOperations();
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
// Before updating, check if binding is already updated
void DescriptorSet::update(const std::vector<uint32_t>& bindingsToUpdate)
{
	std::vector<VkWriteDescriptorSet> writeOperationsToPerform;

	// If empty, update all
	if (bindingsToUpdate.empty())
	{
		for (auto& writeOperation : mWriteOperations)
		{
			// if not already updated then add to writeOperationsToPerform
			if (std::find(mUpdatedBindings.begin(), mUpdatedBindings.end(), writeOperation.dstBinding) == mUpdatedBindings.end())
			{
				writeOperationsToPerform.push_back(writeOperation);
			}
		}
	}
	else 
	{
		// Otherwise just update bindingsToUpdate
		for (auto& writeOperation : mWriteOperations)
		{
			// if update requested then add to writeOperationsToPerform if not already updated
			if (std::find(bindingsToUpdate.begin(), bindingsToUpdate.end(), writeOperation.dstBinding) != bindingsToUpdate.end() &&
				std::find(mUpdatedBindings.begin(), mUpdatedBindings.end(), writeOperation.dstBinding) == mUpdatedBindings.end())
			{
				writeOperationsToPerform.push_back(writeOperation);
			}
		}
	}

	// Update descriptor sets
	if (!writeOperationsToPerform.empty())
	{
		vkUpdateDescriptorSets(mDevice.logicalDevice(),
			static_cast<uint32_t>(writeOperationsToPerform.size()),
			writeOperationsToPerform.data(),
			0,
			nullptr);
	}

	// add list of updated bindings
	for (auto& writeOperation : writeOperationsToPerform)
	{
		mUpdatedBindings.push_back(writeOperation.dstBinding);
	}

}

// Clear updated binding vector
// If a non-empty info vector is passed prepare new write operations
void DescriptorSet::reset(const BindingMap<VkDescriptorImageInfo>& newImageInfos, const BindingMap<VkDescriptorBufferInfo>& newBufferInfos)
{
	mUpdatedBindings.clear();

	// Prepare new write operations
	if (!newImageInfos.empty() && !newBufferInfos.empty())
	{
		mImageInfos = newImageInfos;
		mBufferInfos = newBufferInfos;

		mWriteOperations.clear();
		prepareWriteOperations();
	}
	
}

void DescriptorSet::prepareWriteOperations()
{
	prepareImageWriteOperations();
	prepareBufferWriteOperations();
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
