#pragma once
#include "Common.h"

#include "DescriptorPool.h"


// This class manages a descriptor set once it is allocated by from a pool
// This primarily involves updating descriptors with writes
class DescriptorSet
{
public:
	DescriptorSet(Device& device,
		DescriptorSetLayout& descriptorSetLayout,
		DescriptorPool& descriptorPool,
		const BindingMap<VkDescriptorImageInfo>& imageInfos = {},		// BindingMap image infos and corresponding binding index
		const BindingMap<VkDescriptorBufferInfo>& bufferInfos = {});	// BindingMap with buffer info and corresponding binding index

	// Alternate order
	DescriptorSet(Device& device,
		DescriptorSetLayout& descriptorSetLayout,
		DescriptorPool& descriptorPool,
		const BindingMap<VkDescriptorBufferInfo>& bufferInfos = {},	
		const BindingMap<VkDescriptorImageInfo>& imageInfos = {});
		
	~DescriptorSet() = default;

	// - Getters
	Device& device() const;
	VkDescriptorSet handle() const;
	const BindingMap<VkDescriptorImageInfo>& imageInfos() const;
	const BindingMap<VkDescriptorBufferInfo>& bufferInfos() const;

	// - Management
	void update(const std::vector<uint32_t>& bindingsToUpdate = {});
	void reset(const BindingMap<VkDescriptorImageInfo>& newImageInfos = {},
		const BindingMap<VkDescriptorBufferInfo>& newBufferInfos = {});


private:
	Device& mDevice;
	DescriptorSetLayout& mDescriptorSetLayout;
	DescriptorPool& mDescriptorPool;

	VkDescriptorSet mHandle{ VK_NULL_HANDLE };

	BindingMap<VkDescriptorImageInfo>	mImageInfos;
	BindingMap<VkDescriptorBufferInfo>	mBufferInfos;

	std::vector<VkWriteDescriptorSet> mWriteOperations;
	std::vector<uint32_t> mUpdatedBindings;

	// - Write operation support
	void prepareWriteOperations();
	void prepareImageWriteOperations();
	void prepareBufferWriteOperations();

};

