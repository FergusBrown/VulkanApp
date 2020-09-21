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

// Submit a command buffer (no semaphores)
void Queue::submit(const CommandBuffer& commandBuffer, VkFence fence) const 
{
	// Queue submission information
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer.handle();

	// submit command buffer to queue
    VkResult result = vkQueueSubmit(mHandle, 1, &submitInfo, fence);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit Command Buffer to Queue!");
	}
}

// Submit a command buffer (with semaphores)
void Queue::submit(VkSemaphore waitSemaphore, VkPipelineStageFlags waitStage, VkSemaphore signalSemaphore,const CommandBuffer& commandBuffer, VkFence fence) const
{
	// Queue submission information
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;		// Number of semaphores to wait on
	submitInfo.pWaitSemaphores = &waitSemaphore;									// Stages to check semaphores at
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1;													// Number of command buffers to submit
	submitInfo.pCommandBuffers = &commandBuffer.handle();								// Command buffer to submit
	submitInfo.signalSemaphoreCount = 1;	// Number of semaphores to signal
	submitInfo.pSignalSemaphores = &signalSemaphore;								// Semaphore to signal when the command buffer finishes

	// submit command buffer to queue
	VkResult result = vkQueueSubmit(mHandle, 1, &submitInfo, fence);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit Command Buffer to Queue!");
	}
}

void Queue::present(VkSemaphore waitSemaphore, const Swapchain& swapchain, uint32_t imageIndex) const
{
	VkSwapchainKHR swapchainHandle = swapchain.handle();

	// Create present info
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;						// Number of semaphores to wait on
	presentInfo.pWaitSemaphores = &waitSemaphore;			// Semaphores to wait on
	presentInfo.swapchainCount = 1;							// Number of swapchains to present to
	presentInfo.pSwapchains = &swapchainHandle;				// Swapchains to present images to
	presentInfo.pImageIndices = &imageIndex;				// Index of images in swapchains to present

	// Present image
	VkResult result = vkQueuePresentKHR(mHandle, &presentInfo);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present Swapchain Image!");
	}
}

