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

//QueueFamilyIndices Device::queueFamilyIndices() const
//{
//	return mQueueFamilyIndices;
//}
//
//VkQueue Device::graphicsQueue() const
//{
//	return mGraphicsQueue;
//}
//
//VkQueue Device::presentationQueue() const
//{
//	return mPresentationQueue;
//}

CommandPool& Device::primaryCommandPool()
{
	return *mPrimaryCommandPool;
}

const Queue& Device::queue(uint32_t familyIndex, uint32_t index) const
{
	return mQueues[familyIndex][index];
}

const Queue& Device::getQueueByFlag(VkQueueFlagBits queueFlag, uint32_t index)
{
	for (auto& queueFamily : mQueues)
	{
		auto& firstQueue = queueFamily[0];
		auto& queueProperties = firstQueue.properties();

		if (((queueProperties.queueFlags & queueFlag) == queueFlag) && index < queueProperties.queueCount)
		{
			return queueFamily[index];
		}
	}

	throw std::runtime_error("Could not find the requested queue!");
}

// Request a command buffer for the primary queue
CommandBuffer& Device::requestCommandBuffer(VkCommandBufferLevel level)
{
	return mPrimaryCommandPool->requestCommandBuffer(level);
}

// Allocates a buffer to the device's command pools without incrementing the pool's counter
// This must be submitted and freed with the submitTemporaryCommandBuffer() function
// Use this for one-off copy commands etc.
std::unique_ptr<CommandBuffer> Device::createAndBeginTemporaryCommandBuffer(VkCommandBufferLevel level)
{
	// ALLOCATE COMMAND BUFFER
	std::unique_ptr<CommandBuffer> commandBuffer = std::make_unique<CommandBuffer>(mPrimaryCommandPool->handle(), level);

	// BEGIN RECORDING
	commandBuffer->beginRecording();

	return commandBuffer;
}

// end buffer recording, submits to queue and frees the command buffer
void Device::endAndSubmitTemporaryCommandBuffer(CommandBuffer& commandBuffer)
{
	if (commandBuffer.handle() == VK_NULL_HANDLE)
	{
		return;
	}

	// END RECORDING
	commandBuffer.endRecording();

	// SUBMIT TO QUEUE
	auto& queue = mQueues[commandBuffer.queueFamilyIndex()][0];		// get first queue with appropriate family index

	queue.submit(commandBuffer);
	vkQueueWaitIdle(queue.handle()); // TODO : need to abstract this out by requesting fences on submit

	// CHECK : command buffer should now leave scope and be automatically freed
	// FREE COMMAND BUFFER
	//vkFreeCommandBuffers(mLogicalDevice, mPrimaryCommandPool->handle(), 1, &commandBuffer);
}


//QueueFamilyIndices Device::getQueueFamilies()
//{
//	QueueFamilyIndices indices;
//
//	// Get all Queue Family property info for the given device
//	uint32_t queueFamilyCount = 0;
//	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, nullptr);
//
//	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
//	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, queueFamilyList.data());
//
//	// Go through each queue family and check if it has at least 1 of the required types of queue
//	int i = 0;
//	for (const auto& queueFamily : queueFamilyList)
//	{
//		// queue count indicates how many queus there are in the family
//
//
//		// First check if queue family has at least 1 queue in that family (could have no queue)
//		// Queue can be multiple types define through bitfield. Need tobitwise AND with VK_QUEUE_*_BIT to check if has required type
//		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
//		{
//			indices.graphicsFamily = i;		// If queue family is valid then get index
//		}
//
//		// Check if queue family supports presentation
//		VkBool32 presentationSupport = false;
//		vkGetPhysicalDeviceSurfaceSupportKHR(mPhysicalDevice, i, mSurface, &presentationSupport);
//		// Check if queue is presentation type (can be both graphics and presentation)
//		if (queueFamily.queueCount > 0 && presentationSupport)
//		{
//			indices.presentationFamily = i;
//		}
//
//		if (indices.isValid())
//		{
//			break;
//		}
//
//		++i;
//	}
//
//	return indices;
//}

uint32_t Device::getQueueFamilyIndex(VkQueueFlagBits queueFlag)
{
	// Get all Queue Family property info for the given device
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, queueFamilyList.data());

	// If looking for compute or transfer then we want to check if 
	// there is exclusively a compute or transfer queue

	if (queueFlag == VK_QUEUE_COMPUTE_BIT)
	{
		for (uint32_t i = 0; i < queueFamilyCount; ++i)
		{
			if (queueFamilyList[i].queueFlags & queueFlag == VK_QUEUE_COMPUTE_BIT)
			{
				return i;
			}
		}
	}

	if (queueFlag == VK_QUEUE_TRANSFER_BIT)
	{
		for (uint32_t i = 0; i < queueFamilyCount; ++i)
		{
			if (queueFamilyList[i].queueFlags & queueFlag == VK_QUEUE_TRANSFER_BIT)
			{
				return i;
			}
		}
	}

	// Otherwise check if another queue is suitable
	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		if (queueFamilyList[i].queueFlags & queueFlag)
		{
			return i;
		}
	}

	throw std::runtime_error("Could not find an index for the requested queue family!");
}

VkBool32 Device::checkPresentationSupport(uint32_t queueFamilyIndex)
{
	VkBool32 presentationSupport = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(mPhysicalDevice, queueFamilyIndex, mSurface, &presentationSupport);

	return presentationSupport;
}

// TODO: remove the samplerAnisotropy flag and pass to this function a list of features we want the device to support
bool Device::checkDeviceSuitable(const std::vector<const char*>& deviceExtensions)
{
	// Information about what the device can do (geo shader, tess shader, wide lines, etc)
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(mPhysicalDevice, &deviceFeatures);


	//QueueFamilyIndices indices = getQueueFamilies();

	uint32_t graphicsIndex = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);

	bool extensionsSupported = checkDeviceExtensionSupport(deviceExtensions);


	// TODO: abstract this check to swapchain construction
	/*bool swapChainValid = false;
	if (extensionsSupported)
	{
		SwapChainDetails swapChainDetails = getSwapChainDetails(device);
		swapChainValid = !swapChainDetails.presentationModes.empty() && !swapChainDetails.formats.empty();
	}*/


	//return indices.isValid() && extensionsSupported && swapChainValid && deviceFeatures.samplerAnisotropy;
	//return indices.isValid() && extensionsSupported && deviceFeatures.samplerAnisotropy;
	return checkPresentationSupport(graphicsIndex) && extensionsSupported && deviceFeatures.samplerAnisotropy;
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
	// Get number of queue families
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, queueFamilyList.data());

	// Create infos for all queue families
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	std::vector<std::vector<float>>      queuePriorities(queueFamilyCount);
	for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; ++queueFamilyIndex)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;							// The index of the family to create a queue from
		queueCreateInfo.queueCount = queueFamilyList[queueFamilyIndex].queueCount;		// Number of queues to create
		queuePriorities[queueFamilyIndex].resize(queueFamilyCount, 1.0f);				//
		queueCreateInfo.pQueuePriorities = queuePriorities[queueFamilyIndex].data();	// Give all queues priority of 1 (1 is highest priority)

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

	// Create queue objects
	mQueues.resize(queueFamilyCount);

	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		VkBool32 presentationSupport = checkPresentationSupport(i);

		for (uint32_t j = 0; j < queueFamilyList[i].queueCount; ++j)
		{
			// NOTE: emplace back allows use of custom constructor
			mQueues[i].emplace_back(*this, i, j, queueFamilyList[i], presentationSupport);
		}
	}
}


void Device::createCommandPool()
{
	mPrimaryCommandPool = std::make_unique<CommandPool>(*this, getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT));
}
