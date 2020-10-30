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
class DescriptorResourceReference;
class DescriptorSet;
class DescriptorSetLayout;
class Queue;
class RenderTarget;
struct ShaderResource;


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
	const DescriptorSetLayout& descriptorSetLayout(uint32_t pipelineIndex = 0);
	const DescriptorSet& descriptorSet(uint32_t pipelineIndex = 0, size_t threadIndex = 0);

	// - Frame management
	void reset();

	// - Command Buffers
	CommandBuffer& requestCommandBuffer(const Queue& queue, VkCommandBufferLevel level, size_t threadIndex = 0);

	// - Descriptor Sets
	void createDescriptorSetLayout(std::vector<ShaderResource>& shaderResources, uint32_t pipelineIndex, uint32_t setIndex = 0);
	void createDescriptorSet(uint32_t pipelineIndex, const BindingMap<uint32_t>& imageIndices = {}, const BindingMap<uint32_t>& bufferIndices = {});
	void createDescriptorSet(uint32_t pipelineIndex, DescriptorResourceReference& resourceReference, const BindingMap<uint32_t>& bufferIndices = {});

	// - Buffers
	uint32_t createBuffer(VkDeviceSize bufferSize,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// TODO : add function specialization, at the moment this doesn not work with vectors
	template<typename T>
	void updateBuffer(uint32_t bufferIndex, T& data)
	{
		// Copy VP data
		void* dst = mBuffers[bufferIndex]->map();
		memcpy(dst, &data, sizeof(T));
		mBuffers[bufferIndex]->unmap();
	}

	// -- Synchronisation
	VkFence requestFence();
	VkSemaphore requestSemaphore();
	void wait();

private:
	// Variables
	Device& mDevice;

	struct ThreadData {
		// Command Pools
		std::vector<std::unique_ptr<CommandPool>> commandPools;			// per thread vector of command pools: Each index holds a pool for a different queue type
		
		// Descriptors - each index maps to a pipeline	
		std::unordered_map<uint32_t, std::unique_ptr<DescriptorPool>> descriptorPools;
		std::unordered_map<uint32_t, std::unique_ptr<DescriptorSet>> descriptorSets;
	};

	size_t mThreadCount{ 1 };
	std::unordered_map<size_t, ThreadData> mThreadData;

	// - Render target
	std::unique_ptr<RenderTarget> mRenderTarget;

	// - Descriptor Set Layouts - Index maps to a pipeline
	std::unordered_map<uint32_t, std::unique_ptr<DescriptorSetLayout>> mDescriptorSetLayouts;

	// - Buffers
	// TODO : update this to support dynamic buffers
	std::vector<std::unique_ptr<Buffer>> mBuffers;

	// - Synchronisation
	FencePool mFencePool;
	SemaphorePool mSemaphorePool;

	// - Support
	// -- Command Pools
	std::unique_ptr<CommandPool>& requestCommandPool(const Queue& queue, size_t threadIndex = 0);
};