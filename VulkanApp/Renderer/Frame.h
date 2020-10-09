#pragma once
#include <ctpl_stl.h>

#include "Common.h"

#include "CommandPool.h"
#include "FencePool.h"
#include "SemaphorePool.h"

class Buffer;
class CommandBuffer;
class Device;
class DescriptorPool;
class DescriptorSet;
class DescriptorSetLayout;
class Queue;
class RenderTarget;


// This is a container for data which must be held by every frame
// All operation regarding command buffers and descriptor sets are handled in this class and multithreaded where possible
class Frame
{
public:
	Frame(Device& device, std::unique_ptr<RenderTarget>&& renderTarget, size_t threadCount = 1);
	~Frame() = default;

	// - Getters
	Device& device() const;
	const RenderTarget& renderTarget() const;
	//CommandPool& commandPool(uint32_t threadIndex = 0);

	// - Frame management
	void reset();

	// -- Command Buffers
	CommandBuffer& requestCommandBuffer(const Queue& queue, VkCommandBufferLevel level, size_t threadIndex = 0);

	// TODO : this functionality needs implemented
	// -- Descriptor Sets
	DescriptorSet& requestDescriptorSet(DescriptorSetLayout& descriptorSetLayout,
		const BindingMap<VkDescriptorImageInfo>& imageInfos = {},
		const BindingMap<VkDescriptorBufferInfo>& bufferInfos = {},
		size_t threadIndex = 0);

	// -- Synchronisation
	VkFence requestFence();
	VkSemaphore requestSemaphore();
	void wait();

private:
	// Variables
	Device& mDevice;

	// - Descriptors

	struct ThreadData {
		// Command Pools
		std::vector<std::unique_ptr<CommandPool>> commandPools;			// per thread vector of command pools: Each index holds a pool for a different queue type
		
		// Descriptors			
		std::vector<std::unique_ptr<DescriptorPool>> descriptorPools;
		std::vector<std::unique_ptr<DescriptorSet>> descriptorSets;

		// Buffers
		std::vector<std::unique_ptr<Buffer>> buffers;
	};

	size_t mThreadCount{ 1 };
	std::unordered_map<size_t, ThreadData> mThreadData;

	// - Render target
	std::unique_ptr<RenderTarget> mRenderTarget;

	// - Descriptors
	std::vector<std::unique_ptr<DescriptorPool>> descriptorPools;
	std::vector<std::unique_ptr<DescriptorSet>> descriptorSets;

	// - Thread Pool
	//void createThreadData();

	// - Synchronisation
	FencePool mFencePool;
	SemaphorePool mSemaphorePool;

	// - Support
	// -- Command Pools
	std::unique_ptr<CommandPool>& requestCommandPool(const Queue& queue, size_t threadIndex = 0);

	// -- Descriptor Pools
};

