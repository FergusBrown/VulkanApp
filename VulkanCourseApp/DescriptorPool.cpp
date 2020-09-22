#include "DescriptorPool.h"

#include "Device.h"
#include "DescriptorSetLayout.h"

// Creates a descriptor pool based on a Descriptor Set Layout which can allocate "maxSets" number of sets
DescriptorPool::DescriptorPool(Device& device, const DescriptorSetLayout& descriptorSetLayout, uint32_t maxSets) :
	mDevice(device), mDescriptorSetLayout(descriptorSetLayout), mMaxSets(maxSets)
{
	const auto& layoutBindings = descriptorSetLayout.layoutBindings();

	std::vector<VkDescriptorPoolSize> poolSizes;

	// Fill pool size struct for each binding
	for (auto& binding : layoutBindings)
	{
		VkDescriptorPoolSize poolSize;

		poolSize.type = binding.descriptorType;
		poolSize.descriptorCount = binding.descriptorCount * maxSets;		// Pool size must equal descriptor count * max sets which can be allocated

		poolSizes.push_back(poolSize);
	}

	VkDescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.maxSets = maxSets;											// Maximum number of descriptor sets which can be created from pool
	poolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());		// Amount of Pool Sizes being passed
	poolCreateInfo.pPoolSizes = poolSizes.data();

	// Create Descriptor pool
	VkResult result = vkCreateDescriptorPool(device.logicalDevice(), &poolCreateInfo, nullptr, &mHandle);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Descriptor Pool!");
	}

}

DescriptorPool::~DescriptorPool()
{
	if (mHandle != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(mDevice.logicalDevice(), mHandle, nullptr);
	}
}

Device& DescriptorPool::device()
{
	return mDevice;
}

VkDescriptorPool DescriptorPool::handle() const
{
	return mHandle;
}

const DescriptorSetLayout& DescriptorPool::descriptorSetLayout() const
{
	return mDescriptorSetLayout;
}

uint32_t DescriptorPool::maxSets() const
{
	return mMaxSets;
}

uint32_t DescriptorPool::allocatedSets() const
{
	return mAllocatedSets;
}

void DescriptorPool::reset()
{
	if (mHandle != VK_NULL_HANDLE)
	{
		vkResetDescriptorPool(mDevice.logicalDevice(), mHandle, 0);
	}
}

VkDescriptorSet DescriptorPool::allocate(uint32_t numberOfSets)
{
	if (mAllocatedSets >= mMaxSets || (mAllocatedSets + numberOfSets) >= mMaxSets)
	{
		throw std::runtime_error("Attempting to allocate Descriptor Sets that this Pool has available!");
	}

	VkDescriptorSetLayout layout = mDescriptorSetLayout.handle();

	// Input Attachment Descriptor Set Allocation Info
	VkDescriptorSetAllocateInfo setAllocInfo = {};
	setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocInfo.descriptorPool = mHandle;
	setAllocInfo.descriptorSetCount = numberOfSets;
	setAllocInfo.pSetLayouts = &layout;

	VkDescriptorSet descriptorSetHandle = VK_NULL_HANDLE;

	// Allocate descriptor sets
	VkResult result = vkAllocateDescriptorSets(mDevice.logicalDevice(), &setAllocInfo, &descriptorSetHandle);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate Input Attachment Descriptor Sets!");
	}

	return descriptorSetHandle;
}
