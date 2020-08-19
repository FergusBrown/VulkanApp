#include "Device.h"

Device::Device()
{


}

VkPhysicalDevice Device::getPhysicalDevice() const
{
    return mPhysicalDevice;
}

VkDevice Device::getLogicalDevice() const
{
    return mLogicalDevice;
}

void Device::getPhysicalDevice()
{
	// Enumerate Physical devices the vkInstance can access
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	// If no devices available, then no support Vulkan!
	if (deviceCount == 0)
	{
		throw std::runtime_error("Can't find GPUs that support Vulkan instance!");
	}

	// Get list of Physcial Devices
	std::vector<VkPhysicalDevice> deviceList(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, deviceList.data());

	// Find most suitable device
	// TEMP: atm we just find the first suitable
	for (const auto& device : deviceList)
	{
		if (checkDeviceSuitable(device))
		{
			mainDevice.physicalDevice = device;
			break;
		}
	}

	// Get properties of our new device
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(mainDevice.physicalDevice, &deviceProperties);

	//minUniformBufferOffset = deviceProperties.limits.minUniformBufferOffsetAlignment;
}
