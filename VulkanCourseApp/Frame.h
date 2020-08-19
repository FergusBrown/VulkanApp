#pragma once

#include "Common.h"

#include "RenderTarget.h"


// This is a container for objects which must be held by every frame
// The Frame Object manages creation and destruction of the objects it owns
class Frame
{
public:
	Frame();



private:
	// - Primary Command Buffer
	static VkCommandPool mPrimaryCommandPool;
	VkCommandBuffer mPrimaryCommandBuffer;

	// - Descriptors
	std::vector<VkDescriptorPool> mDescriptorPools;
	std::vector<VkDescriptorSet> mDescriptorSets;

	// - Synchronisation
	VkSemaphore mImageAvailable;
	VkSemaphore mRenderFinished;
	VkFence		mDrawFence;

	// - Per Thread Data
	static size_t threadCount;
	struct ThreadData {
		VkCommandPool commandPool;
		// One secondary command buffer per task
		std::vector<VkCommandBuffer> commandBuffer;
	};
	std::vector<ThreadData> mThreadData;


	// - Render target
	std::unique_ptr<RenderTarget> mRenderTarget;
};

