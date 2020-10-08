#include "PhysicalDevice.h"

PhysicalDevice::PhysicalDevice(VkPhysicalDevice handle) :
	mHandle(handle)
{
	vkGetPhysicalDeviceFeatures(mHandle, &mFeatures);
	vkGetPhysicalDeviceProperties(mHandle, &mProperties);
	vkGetPhysicalDeviceMemoryProperties(mHandle, &mMemoryProperties);
}

PhysicalDevice::PhysicalDevice(PhysicalDevice&& other) :
	mHandle(other.mHandle),
	mFeatures(other.mFeatures),
	mProperties(other.mProperties),
	mMemoryProperties(other.mMemoryProperties)
{
	other.mHandle = VK_NULL_HANDLE;
}

VkPhysicalDevice PhysicalDevice::handle() const
{
	return mHandle;
}

const VkPhysicalDeviceFeatures PhysicalDevice::features() const
{
	return mFeatures;
}

const VkPhysicalDeviceProperties PhysicalDevice::properties() const
{
	return mProperties;
}

const VkPhysicalDeviceMemoryProperties PhysicalDevice::memoryProperties() const
{
	return mMemoryProperties;
}

VkBool32 PhysicalDevice::checkDeviceSuitable(const std::vector<const char*>& requiredExtensions, VkPhysicalDeviceFeatures& requiredFeatures, VkSurfaceKHR presentationSurface)
{
	uint32_t graphicsIndex = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);

	VkBool32 presentationSupported = checkQueueFamilyPresentationSupport(graphicsIndex, presentationSurface);
	VkBool32 extensionsSupported = checkDeviceExtensionSupport(requiredExtensions);
	VkBool32 featuresSupported = checkDeviceFeatureSupport(requiredFeatures);

	return presentationSupported && extensionsSupported && featuresSupported;
}

VkBool32 PhysicalDevice::checkQueueFamilyPresentationSupport(uint32_t queueFamilyIndex, VkSurfaceKHR surface)
{
	VkBool32 presentationSupport = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(mHandle, queueFamilyIndex, surface, &presentationSupport);

	return presentationSupport;
}

uint32_t PhysicalDevice::getQueueFamilyIndex(VkQueueFlagBits queueFlag)
{
	// Get all Queue Family property info for the given device
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(mHandle, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(mHandle, &queueFamilyCount, queueFamilyList.data());

	// If looking for compute or transfer then we want to check if 
	// there is exclusively a compute or transfer queue

	if (queueFlag == VK_QUEUE_COMPUTE_BIT)
	{
		for (uint32_t i = 0; i < queueFamilyCount; ++i)
		{
			if ((queueFamilyList[i].queueFlags & queueFlag) == VK_QUEUE_COMPUTE_BIT)
			{
				return i;
			}
		}
	}

	if (queueFlag == VK_QUEUE_TRANSFER_BIT)
	{
		for (uint32_t i = 0; i < queueFamilyCount; ++i)
		{
			if ((queueFamilyList[i].queueFlags & queueFlag) == VK_QUEUE_TRANSFER_BIT)
			{
				return i;
			}
		}
	}

	// Otherwise check if another queue is suitable
	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		if ((queueFamilyList[i].queueFlags & queueFlag) == queueFlag)
		{
			return i;
		}
	}

	throw std::runtime_error("Could not find an index for the requested queue family!");
}

VkBool32 PhysicalDevice::checkDeviceExtensionSupport(const std::vector<const char*>& deviceExtensions)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(mHandle, nullptr, &extensionCount, nullptr);

	// If no extensions found, return failure
	if (extensionCount == 0)
	{
		return false;
	}

	// Populate lists of extensions
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(mHandle, nullptr, &extensionCount, extensions.data());

	// Check for extension
	for (const auto& deviceExtension : deviceExtensions)
	{
		bool hasExtension = false;
		for (const auto& extension : extensions)
		{
			if (strcmp(deviceExtension, extension.extensionName) == 0)
			{
				hasExtension = true;
				break;
			}
		}

		if (!hasExtension)
		{
			return false;
		}
	}

	return true;
}

// TODO : update this to compare all requested features
VkBool32 PhysicalDevice::checkDeviceFeatureSupport(VkPhysicalDeviceFeatures& requiredFeatures)
{
	if (requiredFeatures.samplerAnisotropy)
	{
		return requiredFeatures.samplerAnisotropy && mFeatures.samplerAnisotropy;
	}
	

	return VK_FALSE;
}
