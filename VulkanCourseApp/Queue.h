#pragma once
#include "Common.h"

#include "Device.h"

class Queue
{
public:
	Queue(Device& device, uint32_t familyIndex, uint32_t index, VkQueueFamilyProperties properties);
	~Queue() = default;

	// - Getters
	const Device& device() const;
	VkQueue handle() const;
	uint32_t familyIndex() const;
	uint32_t index() const;
	const VkQueueFamilyProperties& properties() const;

private:
	Device& mDevice;

	VkQueue mHandle{ VK_NULL_HANDLE };

	uint32_t mFamilyIndex;

	uint32_t mIndex;

	VkQueueFamilyProperties mProperties;
};

