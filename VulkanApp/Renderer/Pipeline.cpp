#include "Pipeline.h"

#include "Device.h"


Pipeline::~Pipeline()
{
	if (mHandle != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(mDevice.logicalDevice(), mHandle, nullptr);
	}
}

VkPipeline Pipeline::handle() const
{
	return mHandle;
}
