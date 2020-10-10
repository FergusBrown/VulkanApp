#include "CommandBuffer.h"

//#include "Buffer.h"
#include "CommandPool.h"
#include "DescriptorSet.h"
#include "Device.h"
#include "Framebuffer.h"
#include "Image.h"
#include "Pipeline.h"
#include "PipelineLayout.h"
#include "RenderTarget.h"
#include "RenderPass.h"

CommandBuffer::CommandBuffer(CommandPool& commandPool, VkCommandBufferLevel level) :
	mCommandPool(commandPool), 
	mLevel(level),
	mMaxPushConstantSize(commandPool.device().physicalDeviceProperties().limits.maxPushConstantsSize)
{

	VkCommandBufferAllocateInfo cbAllocInfo = {};
	cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbAllocInfo.commandPool = commandPool.handle();
	cbAllocInfo.level = level;						// VK_COMMAND_BUFFER_LEVEL_PRIMARY   : buffer you submit directly to the queue. Cannot be called by other buffers.
													// VK_COMMAND_BUFFER_LEVEL_SECONDARY : buffer cannot be called directly. Can be called by other buffers via "vkCmdExecuteCommands" when recording commands in primary buffer
	cbAllocInfo.commandBufferCount = 1;

	// Allocate command buffers and place handles in array of buffers
	VkResult result = vkAllocateCommandBuffers(commandPool.device().logicalDevice(), &cbAllocInfo, &mHandle);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate Command Buffer(s)!");
	}
}

CommandBuffer::~CommandBuffer()
{
	// Free command buffer when out of scope
	if (mHandle != VK_NULL_HANDLE)
	{
		vkFreeCommandBuffers(mCommandPool.device().logicalDevice(), mCommandPool.handle(), 1, &mHandle);
	}
}

const VkCommandBuffer& CommandBuffer::handle() const
{
	return mHandle;
}

const VkCommandBufferLevel CommandBuffer::level() const
{
	return mLevel;
}

uint32_t CommandBuffer::queueFamilyIndex() const
{
	return mCommandPool.queueFamilyIndex();
}

//void CommandBuffer::bindRenderPass(VkRenderPass* renderpassBinding, Framebuffer* framebufferBinding)
//{
//	mRenderPassBinding.renderPass = renderpassBinding;
//	mRenderPassBinding.framebuffer = framebufferBinding;
//}

// TODO : update to work with secondary command buffers
void CommandBuffer::beginRecording(VkCommandBufferUsageFlags flags, CommandBuffer* primaryCommandBuffer)
{
	// Information to begin the command buffer record
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = flags;		// We're only using the command buffer once so set up for one time submit

	if (mLevel == VK_COMMAND_BUFFER_LEVEL_SECONDARY)
	{
		assert(primaryCommandBuffer && "A Primary Command Buffer ptr must be provided in order to begin recording with a Secondary Command Buffer!");
		auto& currentRenderPass = primaryCommandBuffer->currentRenderPass();

		assert((currentRenderPass.renderPass && currentRenderPass.framebuffer) && "Cannot begin recording with a Secondary Command Buffer with no RenderPass binding! Have you called beginRenderPass?");

		// Inheritance create info allows secondary buffers to inherit render pass state
		VkCommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.renderPass = currentRenderPass.renderPass->handle();
		inheritanceInfo.framebuffer = currentRenderPass.framebuffer->handle();
		inheritanceInfo.subpass = 0;	// TODO : this must be changed
		beginInfo.pInheritanceInfo = &inheritanceInfo;
	}

	// Begin recording transfer commands
	VkResult result = vkBeginCommandBuffer(mHandle, &beginInfo);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to start recording a Command Buffer!");
	}
}

// Records command to begin execution of a renderpass
// This should be recorded with a primary command buffer
// TODO : need to set this up so that the renderpass and framebuffer refs can be const
void CommandBuffer::beginRenderPass(const RenderTarget& renderTarget,
	RenderPass& renderPass,
	Framebuffer& framebuffer, 
	const std::vector<VkClearValue>& clearValues, 
	VkSubpassContents subpassContentsRecordingStrategy)
{
	assert(mLevel == VK_COMMAND_BUFFER_LEVEL_PRIMARY && "Must begin render pass using a primary command buffer!");

	// Set bindings
	mRenderPassBinding.renderPass = &renderPass;
	mRenderPassBinding.framebuffer = &framebuffer;

	// Create begin info
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = mRenderPassBinding.renderPass->handle();			// Render pass to begin
	renderPassBeginInfo.framebuffer = mRenderPassBinding.framebuffer->handle();
	renderPassBeginInfo.renderArea.offset = { 0, 0 };							// Start point of render pass in pixels
	renderPassBeginInfo.renderArea.extent = renderTarget.extent();				// Size of region to run render pass on (starting at offset)

	renderPassBeginInfo.pClearValues = clearValues.data();						// List of clear values 
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());

	// Record command to begin Render Pass
	vkCmdBeginRenderPass(mHandle, &renderPassBeginInfo, subpassContentsRecordingStrategy);
	// Subpass contents options	:
	// VK_SUBPASS_CONTENTS_INLINE						: only record to primary command buffers for this subpass
	// VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS	: only record to secondary command buffers for this subpass then execute with primary command buffer
}

void CommandBuffer::bindPipeline(VkPipelineBindPoint bindPoint, const Pipeline& pipeline)
{
	vkCmdBindPipeline(mHandle, bindPoint, pipeline.handle());
}


void CommandBuffer::bindVertexBuffers(uint32_t firstBinding, const std::vector<std::reference_wrapper<const Buffer>>& buffers, const std::vector<VkDeviceSize>& offsets)
{
	// Transform to vector of buffer handles
	std::vector<VkBuffer> bufferHandles(buffers.size(), VK_NULL_HANDLE);
	std::transform(buffers.begin(), buffers.end(), bufferHandles.begin(),
		[](const Buffer& buffer) { return buffer.handle(); });

	// Bind vertex buffers
	vkCmdBindVertexBuffers(mHandle, firstBinding, buffers.size(), bufferHandles.data(), offsets.data());
}

void CommandBuffer::bindIndexBuffer(const Buffer& buffer, VkDeviceSize offset, VkIndexType indexType)
{
	vkCmdBindIndexBuffer(mHandle, buffer.handle(), offset, indexType);
}

void CommandBuffer::bindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, const PipelineLayout& pipelineLayout, uint32_t firstSet, const std::vector<std::reference_wrapper<const DescriptorSet>>& descriptorSets)
{
	// Transform vector to hold descriptor handles
	std::vector<VkDescriptorSet> descriptorHandles(descriptorSets.size(), VK_NULL_HANDLE);
	std::transform(descriptorSets.begin(), descriptorSets.end(), descriptorHandles.begin(),
		[](const DescriptorSet& descriptorSet) { return descriptorSet.handle(); });

	// Bind descriptor sets
	vkCmdBindDescriptorSets(mHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.handle(), 
		firstSet, static_cast<uint32_t>(descriptorHandles.size()), descriptorHandles.data(), 0, nullptr);
}

void CommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t firstInstance)
{
	vkCmdDraw(mHandle, vertexCount, instanceCount, firstIndex, firstInstance);
}

// Assemble primitives using index order from index buffer, instanceCount number of times
void CommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	vkCmdDrawIndexed(mHandle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBuffer::executeCommands(const std::vector<CommandBuffer*>& commandBuffers)
{
	assert(mLevel == VK_COMMAND_BUFFER_LEVEL_PRIMARY && "Command must be executed with a Primary Command Buffer!");

	// Transform to vector of command buffer handles
	std::vector<VkCommandBuffer> commandBufferHandles(commandBuffers.size(), VK_NULL_HANDLE);
	std::transform(commandBuffers.begin(), commandBuffers.end(), commandBufferHandles.begin(),
		[](const CommandBuffer* commandBuffer) { return commandBuffer->handle(); });

	// Execute commands
	vkCmdExecuteCommands(mHandle, static_cast<uint32_t>(commandBufferHandles.size()), commandBufferHandles.data());
}

void CommandBuffer::nextSubpass(VkSubpassContents subpassContentsRecordingStrategy)
{
	vkCmdNextSubpass(mHandle, subpassContentsRecordingStrategy);
}

// TODO : pass in struct to define resource range - currently this will only work for images which match the values here
// TODO : expand functionality to allow for other types of transitions
// Set up image memory barriers and transition image from one layout to another
void CommandBuffer::transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	// Create buffer
	//VkCommandBuffer commandBuffer = beginCommandBuffer(device, commandPool);

	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.oldLayout = oldLayout;									// Layout to transition from
	imageMemoryBarrier.newLayout = newLayout;									// Layout to transition to
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;			// Queue family to transition from
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;			// Queue family to transition to
	imageMemoryBarrier.image = image;											// Image being accessed and modified as part of barrier
	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;	// Aspect of image being altered
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;						// First mip level to start alterations on
	imageMemoryBarrier.subresourceRange.levelCount = 1;							// Number of mip levels to alter starting from baseMipLevel
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;						// First Layer to start alterations on
	imageMemoryBarrier.subresourceRange.layerCount = 1;							// Number of layers to alter starting from baseArrayLayer


	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;

	// If transitioning from new image to image ready to receive data...
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		imageMemoryBarrier.srcAccessMask = 0;									// Memory access stage transition must happen after...
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;		// Memory access stage transition must happen before...

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;		// pipeline stage transition must happen after
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;			// pipeline stage transition must happen before

	}
	// If transitioning from transfer destination to shader readable
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	

	vkCmdPipelineBarrier(
		mHandle,
		srcStage, dstStage,			// Pipeline stages (match to src and dst AccessMasks)
		0,							// Dependency flags
		0, nullptr,					// Memory Barrier count + data
		0, nullptr,					// Buffer Memory Barrier count + data
		1, &imageMemoryBarrier		// Image Memory Batrrier count + data
	);

	//endAndSubmitCommandBuffer(device, commandPool, queue, commandBuffer);
}

// TODO : pass in required subresource values
void CommandBuffer::copyBufferToImage(Buffer& srcBuffer, Image& image)
{
	//VkImageSubresource subresource = image.subresource();

	VkBufferImageCopy imageRegion = {};
	imageRegion.bufferOffset = 0;											// Offset into data
	imageRegion.bufferRowLength = 0;										// Row length of data to calculate data spacing
	imageRegion.bufferImageHeight = 0;										// Image height to calculate data spacing
	imageRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;	// Which aspect of image to copy
	imageRegion.imageSubresource.mipLevel = 0;								// Mipmap level to copy
	imageRegion.imageSubresource.baseArrayLayer = 0;						// Starting array layer (if array)
	imageRegion.imageSubresource.layerCount = 1;							// Number of layers to copy starting at baseArrayLayer
	imageRegion.imageOffset = { 0, 0, 0 };									// Offset into image (as opposed to raw data in buffer offset)
	imageRegion.imageExtent = image.extent();								// Size of region to copy as (x, y, z) values

	// Copy buffer to given image
	vkCmdCopyBufferToImage(mHandle, srcBuffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageRegion);
}

void CommandBuffer::copyBuffer(Buffer& srcBuffer, Buffer& dstBuffer)
{
	// Region of data to copy from and to
	VkBufferCopy bufferCopyRegion = {};
	bufferCopyRegion.srcOffset = 0;				// Copy from start of buffer
	bufferCopyRegion.dstOffset = 0;				// To end of buffer :: This setup directs to copy all of buffer
	bufferCopyRegion.size = srcBuffer.size();

	vkCmdCopyBuffer(mHandle, srcBuffer.handle(), dstBuffer.handle(), 1, &bufferCopyRegion);

}

void CommandBuffer::endRenderPass()
{
	vkCmdEndRenderPass(mHandle);
}

void CommandBuffer::endRecording()
{
	// End command recording
	VkResult result = vkEndCommandBuffer(mHandle);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to stop recording Command Buffer!");
	}
}

const RenderPassBinding& CommandBuffer::currentRenderPass() const
{
	return mRenderPassBinding;
}

// TODO: MOVE TO QUEUE CLASS\?
// TODO : update to allow fences
//void CommandBuffer::submit()
//{
//	// Queue submission information
//	VkSubmitInfo submitInfo = {};
//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//	submitInfo.commandBufferCount = 1;
//	submitInfo.pCommandBuffers = &mHandle;
//
//	uint32_t queueFamilyIndex = mCommandPool.queueFamilyIndex();
//
//
//	// Submit transfer command to transfer queue and wait until it finishes
//	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
//	//vkQueueWaitIdle(queue);			// Avoid submitting many command buffer
//}
