#pragma once
#include "Common.h"

#include "Buffer.h"
class CommandPool;
class DescriptorSet;
class Framebuffer;
class Image;
class Pipeline;
class PipelineLayout;
class RenderTarget;
class RenderPass;

// TODO : modify once renderpass object has been implemented
struct RenderPassBinding
{
	RenderPass* renderPass = nullptr;

	Framebuffer* framebuffer = nullptr;
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
	void beginRecording(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, CommandBuffer* primaryCommandBuffer = nullptr);
	void beginRenderPass(const RenderTarget& renderTarget,
		RenderPass& renderPass,
		Framebuffer& framebuffer,
		const std::vector<VkClearValue>& clearValues,
		VkSubpassContents subpassContentsRecordingStrategy = VK_SUBPASS_CONTENTS_INLINE);
	void bindPipeline(VkPipelineBindPoint bindPoint, const Pipeline& pipeline);

	// TODO : need to update this to be able to take several values
	// TODO : pipeline layout reference should be bound when the pipeline is bound (should retrieve from the render pass object binding (does not exist at the moment))
	template<typename T>
	void pushConstant(const PipelineLayout& pipelineLayout, VkShaderStageFlags shaderStageFlags, const T& value)
	{
		// Check size
		uint32_t size = sizeof(T);
		assert(size <= mMaxPushConstantSize && "Push constant size is greater than 128 bytes!");

		// Push constant to shader stage
		vkCmdPushConstants(mHandle,
			pipelineLayout.handle(),
			shaderStageFlags,		// Stage to push constants to
			0,						// Offset of push constants to update
			size,					// Size of data being pushed	
			&value);				// Actual data being pushed (can be array)
	}

	void bindVertexBuffers(uint32_t firstBinding, const std::vector<std::reference_wrapper<const Buffer>>& buffers, const std::vector<VkDeviceSize>& offsets);
	void bindIndexBuffer(const Buffer& buffer, VkDeviceSize offset, VkIndexType indexType = VK_INDEX_TYPE_UINT32);

	// TODO : update to include dynamic bindings
	void bindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, const PipelineLayout& pipelineLayout, uint32_t firstSet, const std::vector<std::reference_wrapper<const DescriptorSet>>& descriptorSets);


	void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t firstInstance);
	void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);

	void executeCommands(const std::vector<CommandBuffer*>& commandBuffers);

	void nextSubpass(VkSubpassContents subpassContentsRecordingStrategy = VK_SUBPASS_CONTENTS_INLINE);

	// -- Transition and copy operations
	void transitionImageLayout(VkImage image, 
		VkImageLayout oldLayout,
		VkImageLayout newLayout, 
		uint32_t baseMipLevel = 0,
		uint32_t mipLevelCount = 1);
	void copyBufferToImage(Buffer& srcBuffer, Image& image);
	void copyBuffer(Buffer& srcBuffer, Buffer& dstBuffer);
	void blitImage(Image& srcImage, 
		VkImageLayout srcLayout,
		Image& dstImage, 
		VkImageLayout dstLayout,
		VkImageBlit blitDescription,
		uint32_t regionCount = 1,
		VkFilter filter = VK_FILTER_LINEAR);

	void endRenderPass();
	void endRecording();
	//void submit();

private:
	CommandPool& mCommandPool;

	VkCommandBuffer mHandle{ VK_NULL_HANDLE };

	const VkCommandBufferLevel mLevel;

	const uint32_t mMaxPushConstantSize;

	RenderPassBinding mRenderPassBinding;

	const RenderPassBinding& currentRenderPass() const;


};
