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
	void beginRecording(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	void beginRenderPass(const RenderTarget& renderTarget,
		const VkRenderPass& renderPassBinding,
		const Framebuffer& framebufferBinding,
		const std::vector<VkClearValue>& clearValues,
		VkSubpassContents subpassContentsRecordingStrategy);

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

	RenderPassBinding mRenderPassBinding;


};

