#pragma once
#include "Common.h"

#include "DescriptorPool.h"

/// *** BindingMap ***
/// Top layer key maps to the descriptor set binding
/// Bottom layer key maps to a descriptor within the array of descriptors in the binding
template <typename T>
using BindingMap = std::map<uint32_t, std::map<uint32_t, T>>;

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
		
	~DescriptorSet() = default;

	// - Getters
	Device& device() const;
	VkDescriptorSet handle() const;
	const BindingMap<VkDescriptorImageInfo>& imageInfos() const;
	const BindingMap<VkDescriptorBufferInfo>& bufferInfos() const;

	// - Management
	void update(const std::vector<uint32_t>& bindingsToUpdate = {});
	void reset(const BindingMap<VkDescriptorImageInfo>& newImageInfos = {},
		const BindingMap<VkDescriptorImageInfo>& newBufferInfos = {});
private:
	Device& mDevice;
	DescriptorSetLayout& mDescriptorSetLayout;
	DescriptorPool& mDescriptorPool;

	VkDescriptorSet mHandle{ VK_NULL_HANDLE };

	BindingMap<VkDescriptorImageInfo>	mImageInfos;
	BindingMap<VkDescriptorBufferInfo>	mBufferInfos;

	std::vector<VkWriteDescriptorSet> mWriteOperations;

	void prepareImageWriteOperations();
	void prepareBufferWriteOperations();

};

