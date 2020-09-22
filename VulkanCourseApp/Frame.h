#pragma once
#include <ctpl_stl.h>

#include "Common.h"

class CommandPool;
class CommandBuffer;
class Device;
class DescriptorPool;
class DescriptorSet;
class Queue;
class RenderTarget;



// This is a container for data which must be held by every frame
// All operation regarding command buffers and descriptor sets are handled in this class and multithreaded where possible
class Frame
{
public:
	Frame(Device& device, std::unique_ptr<RenderTarget>&& renderTarget, uint32_t threadCount = 1);
	~Frame() = default;

	// - Getters
	Device& device() const;
	const RenderTarget& renderTarget() const;
	//CommandPool& commandPool(uint32_t threadIndex = 0);

	// - Frame management
	void reset();

	// - Command Buffer management
	CommandBuffer& requestCommandBuffer(const Queue& queue, VkCommandBufferLevel level, size_t threadIndex = 0);

private:
	// Variables
	Device& mDevice;

	// - Descriptors
	/*std::vector<VkDescriptorPool> mDescriptorPools;
	std::vector<VkDescriptorSet> mDescriptorSets;*/

	// - Synchronisation
	// TODO : add fence and semaphore pool
	/*VkSemaphore mImageAvailable;
	VkSemaphore mRenderFinished;
	VkFence		mDrawFence;*/

	// - Thread Pool
	//uint32_t mThreadCount;
	//ctpl::thread_pool mThreadPool;

	struct ThreadData {
		std::vector<std::unique_ptr<CommandPool>> commandPools;			// per thread vector of command pools: Each index holds a pool for a different queue type
		//std::vector<std::unique_ptr<DescriptorPool>> descriptorPools;
		//std::vector<std::unique_ptr<DescriptorSet>> descriptorSets;
	};

	std::vector<ThreadData> mThreadData;

	// - Render target
	std::unique_ptr<RenderTarget> mRenderTarget;

	// - Descriptors
	std::vector<std::unique_ptr<DescriptorPool>> descriptorPools;
	std::vector<std::unique_ptr<DescriptorSet>> descriptorSets;

	// - Thread Pool
	//void createThreadData();

	// - Command Pool
	std::unique_ptr<CommandPool>& requestCommandPool(const Queue& queue, size_t threadIndex = 0);

	// - Synchronisation
	//void createSynchronisation();
};

