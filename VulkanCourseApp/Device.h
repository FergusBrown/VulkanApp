#pragma once

#include "Common.h"

// Contains logical device and associated
class Device
{
public:
	// Create logical device based on list of extensions
	Device(VkPhysicalDevice& physicalDevice, );

	const VkPhysicalDevice& getPhysicalDevice() const;
	VkDevice getLogicalDevice() const;

private:
	VkPhysicalDevice& mPhysicalDevice;
	VkDevice mLogicalDevice;

	void createLogicalDevice();

};

