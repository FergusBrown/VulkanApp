#pragma once
#include "Common.h"

//#include "CommandPool.h"
#include "Image.h"
#include "Buffer.h"
#include "Framebuffer.h"

// TODO : modify once renderpass object has been implemented
struct RenderPassBinding
{
	const VkRenderPass* renderPass;

	const Framebuffer* framebuffer;
};

// Class handles all command buffer operations
class CommandBuffer
{
public:
	CommandBuffer(CommandPool& commandPool, VkCommandBufferLevel level);
	~CommandBuffer();

	// - Getters
	const VkCommandBuffer& handle() const;
	const VkCommandBufferLevel level() const;
	uint32_t queueFamilyIndex() const;

	// - Setters
	/*void bindRenderPass(VkRenderPass* renderpassBinding, Framebuffer* framebufferBinding);*/

	// - Command buffer operations
	void beginRecording(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT);
	void beginRenderPass(const RenderTarget& renderTarget,
		const VkRenderPass& renderPassBinding,
		const Framebuffer& framebufferBinding,
		const std::vector<VkClearValue>& clearValues,
		VkSubpassContents subpassContentsRecordingStrategy = VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
	void bindPipeline(VkPipelineBindPoint bindPoint, VkPipeline& pipeline);

	// TODO : need to update this to be able to take several values
	// TODO : pipeline layout reference should be bound when the pipeline is bound (should retrieve from the render pass object binding (does not exist at the moment))
	template<typename T>
	void pushConstant(VkPipelineLayout pipelineLayout, VkShaderStageFlags shaderStageFlags, const T& value)
	{
rtex buffer befor		// Check size
		uint32_t size = sizeof(T);
		assert(size <= mMaxPushConstantSize && "Push constant size is greater than 128 bytes!");

		// Push constant to shader stage
		vkCmdPushConstants(mHandle,
			pipelineLayout,
			shaderStageFlags,		// Stage to push constants to
			0,						// Offset of push constants to update
			size,					// Size of data being pushed	
			value);					// Actual data being pushed (can be array)
	}

	void bindVertexBuffers(uint32_t firstBinding, const std::vector<std::reference_wrapper<const Buffer>>& buffers, const std::vector<VkDeviceSize>& offsets);
	void bindIndexBuffer(const Buffer& buffer, VkDeviceSize offset, VkIndexType indexType = VK_INDEX_TYPE_UINT32);

	// -- Transition and copy operations
	void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(Buffer& srcBuffer, Image& image);
	void copyBuffer(Buffer& srcBuffer, Buffer& dstBuffer);

	void endRecording();
	//void submit();

private:
	CommandPool& mCommandPool;

	VkCommandBuffer mHandle{ VK_NULL_HANDLE };

	const VkCommandBufferLevel mLevel;

	const uint32_t mMaxPushConstantSize;

	RenderPassBinding mRenderPassBinding;


};
