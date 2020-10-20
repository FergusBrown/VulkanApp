#include "Frame.h"

#include "Buffer.h"
#include "CommandBuffer.h"
#include "DescriptorPool.h"
#include "DescriptorResourceReference.h"
#include "DescriptorSet.h"
#include "DescriptorSetLayout.h"
#include "Device.h"
#include "ImageView.h"
#include "Queue.h"
#include "RenderTarget.h"

Frame::Frame(Device& device, std::unique_ptr<RenderTarget>&& renderTarget, size_t threadCount) :
	mDevice(device), mRenderTarget(std::move(renderTarget)),
	mFencePool(device), mSemaphorePool(device),
	mThreadCount(threadCount)
{

}

Device& Frame::device() const
{
	return mDevice;
}

const RenderTarget& Frame::renderTarget() const
{
	return *mRenderTarget;
}

const DescriptorSetLayout& Frame::descriptorSetLayout(uint32_t pipelineIndex)
{
	return *mDescriptorSetLayouts[pipelineIndex];
}

const DescriptorSet& Frame::descriptorSet(uint32_t pipelineIndex, size_t threadIndex)
{
	return *mThreadData[threadIndex].descriptorSets[pipelineIndex];
}

void Frame::reset()
{
	// Reset synchronisation pools
	mFencePool.reset();
	mSemaphorePool.reset();

	// Reset thread data
	for (size_t i = 0; i < mThreadCount; ++i)
	{
		auto& thread = mThreadData[i];
		for (auto& pool : thread.commandPools)
		{
			pool->reset();
		}
	}
}

CommandBuffer& Frame::requestCommandBuffer(const Queue& queue, VkCommandBufferLevel level, size_t threadIndex)
{
	auto& commandPool = requestCommandPool(queue, threadIndex);

	return commandPool->requestCommandBuffer(level);
}

// Creates a descriptor set layout for resources associated with the frame. 
// setIndex should default to 0 so that the "per view" descriptor set is always bound to set 0 in shaders
void Frame::createDescriptorSetLayout(std::vector<ShaderResource>& shaderResources, uint32_t pipelineIndex, uint32_t setIndex)
{
	// Create descriptor set layout
	mDescriptorSetLayouts[pipelineIndex] = std::make_unique<DescriptorSetLayout>(mDevice, setIndex, shaderResources);

	// Create associted descriptor pool per thread
	for (size_t i = 0; i < mThreadCount; ++i)
	{
		mThreadData[i].descriptorPools[pipelineIndex] = std::make_unique<DescriptorPool>(mDevice, *mDescriptorSetLayouts[pipelineIndex], 1);
	}
}

// Create resource references based on the provided indices to create image and buffer infos
// These can then be used to create the "Per Frame" descriptor set for the provided 
void Frame::createDescriptorSet(uint32_t pipelineIndex, const BindingMap<uint32_t>& imageIndices, const BindingMap<uint32_t>& bufferIndices)
{
	// Bind image and buffers to resource reference
	DescriptorResourceReference descriptorSetResourceReference;
	std::vector<uint32_t> bindingsToUpdate{};

	// Bind images
	
	auto& targetImages = mRenderTarget->imageViews();
	for (auto& binding : imageIndices)
	{
		uint32_t bindingIndex = binding.first;
		auto& bindingContents = binding.second;

		bindingsToUpdate.push_back(bindingIndex);

		for (auto& descriptor : bindingContents)
		{
			uint32_t descriptorIndex = descriptor.first;
			uint32_t imageIndex = descriptor.second;

			// Create descriptor resource reference and genereate an image info
			auto& imageView = (*mRenderTarget).mImageViews[imageIndex];
			descriptorSetResourceReference.bindInputImage(imageView, bindingIndex, descriptorIndex);
		}
	}

	// Bind buffers
	for (auto& binding : bufferIndices)
	{
		uint32_t bindingIndex = binding.first;
		auto& bindingContents = binding.second;

		bindingsToUpdate.push_back(bindingIndex);

		for (auto& descriptor : bindingContents)
		{
			uint32_t descriptorIndex = descriptor.first;
			uint32_t bufferIndex = descriptor.second;

			// Create descriptor resource reference and genereate an image info
			auto& buffer = *mBuffers[bufferIndex];
			descriptorSetResourceReference.bindBuffer(buffer,
				0,
				buffer.size(),
				bindingIndex,
				descriptorIndex);
		}
	}

	// Generate image and buffer infos
	BindingMap<VkDescriptorImageInfo> imageInfos;
	BindingMap<VkDescriptorBufferInfo> bufferInfos;

	descriptorSetResourceReference.generateDescriptorInfos(imageInfos, bufferInfos);

	// Create descriptor set per thread
	for (size_t i = 0; i < mThreadCount; ++i)
	{
		auto& descriptorPool = *mThreadData[i].descriptorPools[pipelineIndex];
		mThreadData[i].descriptorSets[pipelineIndex] = std::make_unique<DescriptorSet>(mDevice, *mDescriptorSetLayouts[pipelineIndex], descriptorPool, imageInfos, bufferInfos);
		mThreadData[i].descriptorSets[pipelineIndex]->update(bindingsToUpdate);
	}
}

//void Frame::createDescriptorSet(uint32_t pipelineIndex, const BindingMap<VkDescriptorImageInfo>& imageInfos, const BindingMap<VkDescriptorBufferInfo>& bufferInfos)
//{
//	// Create descriptor set per thread
//	for (size_t i = 0; i < mThreadCount; ++i)
//	{
//		auto& descriptorPool = *mThreadData[i].descriptorPools[pipelineIndex];
//		mThreadData[i].descriptorSets[pipelineIndex] = std::make_unique<DescriptorSet>(mDevice, mDescriptorSetLayouts[pipelineIndex], descriptorPool, imageInfos, bufferInfos);
//	}
//}

uint32_t Frame::createBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
	mBuffers.push_back(std::make_unique<Buffer>(mDevice, bufferSize, usage, properties));

	return static_cast<uint32_t>(mBuffers.size() - 1);
}

VkFence Frame::requestFence()
{
	return mFencePool.requestFence();
}

VkSemaphore Frame::requestSemaphore()
{
	return mSemaphorePool.requestSemaphore();
}

void Frame::wait()
{
	VkResult result = mFencePool.wait();
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to wait for Fence(s) to signal!");
	}
}


// Check if a pool for the requested queue exists
// If it does exist then return it
// Otherwise create the rquested pool
std::unique_ptr<CommandPool>& Frame::requestCommandPool(const Queue& queue, size_t threadIndex)
{
	auto& commandPools = mThreadData[threadIndex].commandPools;
	
	// Return pool if it exists
	for (auto& pool : commandPools)
	{
		if (pool->queueFamilyIndex() == queue.familyIndex())
		{
			return pool;
		}
	}

	// Create and return requested pool
	commandPools.push_back(std::make_unique<CommandPool>(mDevice, queue.familyIndex()));

	return commandPools.back();
}
