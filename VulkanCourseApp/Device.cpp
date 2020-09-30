#include "Device.h"

#include "CommandBuffer.h"
#include "CommandPool.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "Queue.h"

Device::Device(Instance& instance, VkSurfaceKHR surface, const std::vector<const char*>& requiredExtensions, VkPhysicalDeviceFeatures& requiredFeatures)
	:mSurface(surface)
{
	getPhysicalDevice(instance.handle(), requiredExtensions, requiredFeatures);
	createLogicalDevice(requiredExtensions, requiredFeatures);
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
    return mPhysicalDevice->handle();
}

VkDevice Device::logicalDevice() const
{
    return mLogicalDevice;
}


CommandPool& Device::primaryCommandPool()
{
	return *mPrimaryCommandPool;
}

const Queue& Device::queue(uint32_t familyIndex, uint32_t index) const
{
	return mQueues[familyIndex][index];
}

// TODO : abstract to physicald evice class
const VkPhysicalDeviceProperties& Device::physicalDeviceProperties()
{
	VkPhysicalDeviceProperties properties;
	
	vkGetPhysicalDeviceProperties(mPhysicalDevice->handle(), &properties);
	
	return properties;
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
	std::unique_ptr<CommandBuffer> commandBuffer = std::make_unique<CommandBuffer>(*mPrimaryCommandPool, level);

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




uint32_t Device::getQueueFamilyIndex(VkQueueFlagBits queueFlag)
{
	return mPhysicalDevice->getQueueFamilyIndex(queueFlag);
}

void Device::getPhysicalDevice(VkInstance instance, const std::vector<const char*>& requiredExtensions, VkPhysicalDeviceFeatures& requiredFeatures)
{
	// Enumerate Physical devices the vkInstance can access
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	// If no devices available, then no support Vulkan!
	if (deviceCount == 0)
	{
		throw std::runtime_error("Can't find GPUs that support a Vulkan instance!");
	}

	// Get list of Physical Devices
	std::vector<VkPhysicalDevice> deviceList(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, deviceList.data());

	// Find first suitable device
	// TODO: this just finds first suitable, rework to find most suitable of those available
	for (const auto& deviceHandle : deviceList)
	{
		PhysicalDevice physicalDevice(deviceHandle);

		if (physicalDevice.checkDeviceSuitable(requiredExtensions, requiredFeatures, mSurface))
		{
			mPhysicalDevice = std::make_unique<PhysicalDevice>(std::move(physicalDevice));
			return;
		}
	}

	throw std::runtime_error("A physical device which supports the required extensions does not exist!");

}

// Create logical device and associated queues
void Device::createLogicalDevice(const std::vector<const char*>& requiredExtensions, VkPhysicalDeviceFeatures& requiredFeatures)
{
	// Get number of queue families
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice->handle(), &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice->handle(), &queueFamilyCount, queueFamilyList.data());

	// Create infos for all queue families
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	std::vector<std::vector<float>>      queuePriorities(queueFamilyCount);
	for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; ++queueFamilyIndex)
	{
		uint32_t queueCount = queueFamilyList[queueFamilyIndex].queueCount;

		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;							// The index of the family to create a queue from
		queueCreateInfo.queueCount = queueCount;										// Number of queues to create
		queuePriorities[queueFamilyIndex].resize(queueCount, 1.0f);				//
		queueCreateInfo.pQueuePriorities = queuePriorities[queueFamilyIndex].data();	// Give all queues priority of 1 (1 is highest priority)

		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Information to create logical device ( sometimes called "device")
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());				// Number of Queue Create Infos
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();										// List of queue create infos so device can create required queues
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());			// Number of enabled logical device extensions
	deviceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();									// List of enabled logical device extensions

	//// Physical Device Features the Logical Device will be using
	//VkPhysicalDeviceFeatures deviceFeatures = {};
	//deviceFeatures.samplerAnisotropy = VK_TRUE;		// Enable Anisotropy

	deviceCreateInfo.pEnabledFeatures = &requiredFeatures;					// Physical Device features Logical Device will use


	// Create the logical device for the given physical device
	VkResult result = vkCreateDevice(mPhysicalDevice->handle(), &deviceCreateInfo, nullptr, &mLogicalDevice);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Logical Device!");
	}

	// Create queue objects
	mQueues.resize(queueFamilyCount);

	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		VkBool32 presentationSupport = mPhysicalDevice->checkQueueFamilyPresentationSupport(i, mSurface);

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
