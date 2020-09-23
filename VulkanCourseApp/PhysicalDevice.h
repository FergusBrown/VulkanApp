#pragma once
#include "Common.h"

class PhysicalDevice
{
public:
	PhysicalDevice(VkPhysicalDevice handle);
	~PhysicalDevice() = default;
	PhysicalDevice(const PhysicalDevice&) = delete;
	PhysicalDevice(PhysicalDevice&& other);

	// - Getters
	VkPhysicalDevice handle() const;
	const VkPhysicalDeviceFeatures features() const;
	const VkPhysicalDeviceProperties properties() const;
	const VkPhysicalDeviceMemoryProperties memoryProperties() const;

	VkBool32 checkDeviceSuitable(const std::vector<const char*>& requiredExtensions, VkPhysicalDeviceFeatures& requiredFeatures, VkSurfaceKHR presentationSurface);
	VkBool32 checkQueueFamilyPresentationSupport(uint32_t queueFamilyIndex, VkSurfaceKHR surface);
	uint32_t getQueueFamilyIndex(VkQueueFlagBits queueFlag);
	

private:
	VkPhysicalDevice mHandle{ VK_NULL_HANDLE };

	VkPhysicalDeviceFeatures mFeatures;

	VkPhysicalDeviceProperties mProperties;

	VkPhysicalDeviceMemoryProperties mMemoryProperties;

	//std::vector<VkQueueFamilyProperties> mQueueFamilyProperties;

	VkBool32 checkDeviceExtensionSupport(const std::vector<const char*>& requiredExtensions);
	VkBool32 checkDeviceFeatureSupport(VkPhysicalDeviceFeatures& requiredFeatures);
};

