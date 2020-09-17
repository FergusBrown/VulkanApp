#pragma once
#include "Common.h"

#include "Device.h"

class Queue
{
public:
	Queue(Device& device, uint32_t familyIndex, uint32_t index, VkQueueFamilyProperties properties, VkBool32 presentationSupport);
	~Queue() = default;

	// - Getters
	const Device& device() const;
	VkQueue handle() const;
	uint32_t familyIndex() const;
	uint32_t index() const;
	VkBool32 presentationSupport() const;
	const VkQueueFamilyProperties& properties() const;

	// - Management
	void submit(CommandBuffer& commandBuffer, VkFence fence = VK_NULL_HANDLE); // TODO : need to add fence functionality here

private:
	Device& mDevice;

	VkQueue mHandle{ VK_NULL_HANDLE };

	uint32_t mFamilyIndex;

	uint32_t mIndex;

	VkBool32 mPresentationSupport;

	VkQueueFamilyProperties mProperties;
};

