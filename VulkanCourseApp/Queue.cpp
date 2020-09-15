#include "Queue.h"

Queue::Queue(Device& device, uint32_t familyIndex, uint32_t index, VkQueueFamilyProperties properties, VkBool32 presentationSupport) :
    mDevice(device), mFamilyIndex(familyIndex), mIndex(index), mProperties(properties), mPresentationSupport(presentationSupport)
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

VkBool32 Queue::presentationSupport() const
{
    return mPresentationSupport;
}

const VkQueueFamilyProperties& Queue::properties() const
{
    return mProperties;
}
