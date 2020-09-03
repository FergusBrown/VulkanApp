#pragma once
#include "ctpl_stl.h"

#include "Common.h"

#include "Device.h"
#include "RenderTarget.h"
#include "CommandPool.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"

// This is a container for data which must be held by every frame
// All operation regarding command buffers and descriptor sets are handled in this class and multithreaded where possible
class Frame
{
public:
	Frame(Device& device, std::unique_ptr<RenderTarget>&& renderTarget, uint32_t threadCount = 1);
	~Frame();

	// - Getters
	Device& device() const;

	// Frame management
	void reset();

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
	uint32_t mThreadCount;
	ctpl::thread_pool mThreadPool;

	struct ThreadData {
		std::unique_ptr<CommandPool> commandPool;
		std::vector<std::unique_ptr<DescriptorPool>> descriptorPools;
		std::vector<std::unique_ptr<DescriptorSet>> descriptorSets;
	};
	std::vector<ThreadData> mThreadData;

	// - Render target
	std::unique_ptr<RenderTarget> mRenderTarget;

	// Functions
	// - Command Buffers
	// -- Primary
	/*void createPrimaryCommandPool();
	void createPrimaryCommandBuffer();*/

	// - Thread pool management
	void createThreadData();

	// - Synchronisation
	//void createSynchronisation();
};

