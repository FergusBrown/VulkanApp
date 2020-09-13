#include "Queue.h"

Queue::Queue(Device& device, uint32_t familyIndex, uint32_t index, VkQueueFamilyProperties properties) :
    mDevice(device), mFamilyIndex(familyIndex), mIndex(index), mProperties(properties)
{
    vkGetDeviceQueue(device.logicalDevice(), familyIndex, index, &mHandle);
}

const Device& Queue::device() const
{
    return mDevice;
}

VkQueue Queue::handle() const
{
    return mHandle;
}

uint32_t Queue::familyIndex() const
{
    return mFamilyIndex;
}

uint32_t Queue::index() const
{
    return mIndex;
}

const VkQueueFamilyProperties& Queue::properties() const
{
    return mProperties;
}
