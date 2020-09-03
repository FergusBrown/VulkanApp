#pragma once
#include "Common.h"

#include "Device.h"
#include "DescriptorSetLayout.h"

class DescriptorPool
{
public:
	DescriptorPool(Device& device, const DescriptorSetLayout& descriptorSetLayout, uint32_t maxSets);
	~DescriptorPool();

	// - Getters
	Device& device();
	VkDescriptorPool handle() const;
	const DescriptorSetLayout& descriptorSetLayout() const;
	uint32_t maxSets() const;
	uint32_t allocatedSets() const;

	// - Pool Management
	void reset();
	VkDescriptorSet allocate(uint32_t numberOfSets = 1);

private:
	Device& mDevice;
	const DescriptorSetLayout& mDescriptorSetLayout;

	VkDescriptorPool mHandle{VK_NULL_HANDLE};

	uint32_t mMaxSets;
	uint32_t mAllocatedSets{ 0 };


};

