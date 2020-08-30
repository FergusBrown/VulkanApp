#include "Device.h"



Device::Device(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions)
	:mSurface(surface)
{
	getPhysicalDevice(instance, deviceExtensions);
	createLogicalDevice(deviceExtensions);
	createCommandPool();
}

Device::~Device()
{
	vkDeviceWaitIdle(mLogicalDevice);
	vkDestroyDevice(mLogicalDevice, nullptr);
}

// NOTE: all these types are just pointers so return by value
VkPhysicalDevice Device::physicalDevice() const
{
    return mPhysicalDevice;
}

VkDevice Device::logicalDevice() const
{
    return mLogicalDevice;
}

QueueFamilyIndices Device::queueFamilyIndices() const
{
	return mQueueFamilyIndices;
}

VkQueue Device::graphicsQueue() const
{
	return mGraphicsQueue;
}

VkQueue Device::presentationQueue() const
{
	return mPresentationQueue;
}

CommandPool& Device::primaryCommandPool()
{
	return *mPrimaryCommandPool;
}

QueueFamilyIndices Device::getQueueFamilies()
{
	QueueFamilyIndices indices;

	// Get all Queue Family property info for the given device
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, queueFamilyList.data());

	// Go through each queue family and check if it has at least 1 of the required types of queue
	int i = 0;
	for (const auto& queueFamily : queueFamilyList)
	{
		// queue count indicates how many queus there are in the family


		// First check if queue family has at least 1 queue in that family (could have no queue)
		// Queue can be multiple types define through bitfield. Need tobitwise AND with VK_QUEUE_*_BIT to check if has required type
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;		// If queue family is valid then get index
		}

		// Check if queue family supports presentation
		VkBool32 presentationSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(mPhysicalDevice, i, mSurface, &presentationSupport);
		// Check if queue is presentation type (can be both graphics and presentation)
		if (queueFamily.queueCount > 0 && presentationSupport)
		{
			indices.presentationFamily = i;
		}

		if (indices.isValid())
		{
			break;
		}

		++i;
	}

	return indices;
}

// TODO: remove the samplerAnisotropy flag and pass to this function a list of features we want the device to support
bool Device::checkDeviceSuitable(const std::vector<const char*>& deviceExtensions)
{
	// Information about what the device can do (geo shader, tess shader, wide lines, etc)
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(mPhysicalDevice, &deviceFeatures);


	QueueFamilyIndices indices = getQueueFamilies();

	bool extensionsSupported = checkDeviceExtensionSupport(deviceExtensions);


	// TODO: abstract this check to swapchain construction
	/*bool swapChainValid = false;
	if (extensionsSupported)
	{
		SwapChainDetails swapChainDetails = getSwapChainDetails(device);
		swapChainValid = !swapChainDetails.presentationModes.empty() && !swapChainDetails.formats.empty();
	}*/


	//return indices.isValid() && extensionsSupported && swapChainValid && deviceFeatures.samplerAnisotropy;
	return indices.isValid() && extensionsSupported && deviceFeatures.samplerAnisotropy;
}

bool Device::checkDeviceExtensionSupport(const std::vector<const char*>& deviceExtensions)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr, &extensionCount, nullptr);

	// If no extensions found, return failure
	if (extensionCount == 0)
	{
		return false;
	}

	// Populate lists of extensions
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr, &extensionCount, extensions.data());

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

void Device::getPhysicalDevice(VkInstance instance, const std::vector<const char*>& deviceExtensions)
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
	// TODO: this just finds first suitable, rewrite to find all suitable
	for (const auto& device : deviceList)
	{
		if (checkDeviceSuitable(deviceExtensions))
		{
			mPhysicalDevice = device;
			break;
		}
	}

	throw std::runtime_error("A physical device which supports the required extensions does not exist!");

}

void Device::createLogicalDevice(const std::vector<const char*>& deviceExtensions)
{
	// Get the queue family indices for the chosen physical device
	QueueFamilyIndices indices = getQueueFamilies();

	// Vector for queue creation information, and set for family indices
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> queueFamilyIndices = { indices.graphicsFamily, indices.presentationFamily };

	// Queue the logical device needs to create and info to do so (Only 1 for now)
	for (int queueFamilyIndex : queueFamilyIndices)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;				// The index of the family to create a queue from
		queueCreateInfo.queueCount = 1;											// Number of queues to create
		float priority = 1.0f;													//
		queueCreateInfo.pQueuePriorities = &priority;							// Vulkan needs to know how to handle multiple queues, so decide priority (1 is highest priority)

		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Information to create logical device ( sometimes called "device")
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());				// Number of Queue Create Infos
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();										// List of queue create infos so device can create required queues
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());			// Number of enabled logical device extensions
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();									// List of enabled logical device extensions

	// Physical Device Features the Logical Device will be using
	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;		// Enable Anisotropy

	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;					// Physical Device features Logical Device will use


	// Create the logical device for the given physical device
	VkResult result = vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mLogicalDevice);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Logical Device!");
	}

	// Queues are created at the same time as the device...
	// So we want to handle queues
	// From given logical device, of given Queue Family, of given Queue Index (0 since only one queue), place reference in given Vkqueue
	vkGetDeviceQueue(mLogicalDevice, indices.graphicsFamily, 0, &mGraphicsQueue);
	vkGetDeviceQueue(mLogicalDevice, indices.presentationFamily, 0, &mPresentationQueue);
}
// TODO
void Device::createCommandPool()
{
}
