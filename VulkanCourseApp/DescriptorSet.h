#pragma once
#include "Common.h"

#include "DescriptorPool.h"

class DescriptorSet
{
public:
	DescriptorSet(Device& device,
		DescriptorPool& descriptorPool,
		std::unordered_map<uint32_t, VkDescriptorImageInfo>& imageInfos = {},		// hashmap with image info and corresponding binding index
		std::unordered_map<uint32_t, VkDescriptorBufferInfo>& bufferInfos = {});	// hashmap with buffer info and corresponding binding index
		
	~DescriptorSet() = default;

	// - Getters
	Device& device() const;
	VkDescriptorSet handle() const;

	// Management
	void update(const std::vector<uint32_t>& bindingsToUpdate = {});
private:
	Device& mDevice;
	DescriptorPool& mDescriptorPool;

	VkDescriptorSet mHandle{ VK_NULL_HANDLE };

	std::unordered_map<uint32_t, VkDescriptorImageInfo>		mImageInfos;
	std::unordered_map<uint32_t, VkDescriptorBufferInfo>	mBufferInfos;
};

