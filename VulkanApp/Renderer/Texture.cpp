#include "Texture.h"

#include "Buffer.h"
#include "CommandBuffer.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include "Image.h"
#include "ImageView.h"

uint32_t Texture::ID = 0;

Texture::Texture(Device& device, 
	void* textureData, 
	int width,
	int height, 
	VkDeviceSize imageSize, 
	VkFormat imageFormat) :
	mDevice(device), mID(ID)
{
	createTextureImage(textureData, static_cast<uint32_t>(width), static_cast<uint32_t>(height), imageSize, imageFormat);

	mImageView = std::make_unique<ImageView>(*mImage, VK_IMAGE_VIEW_TYPE_2D);

	++ID;
}

const Device& Texture::device() const
{
	return mDevice;
}

const ImageView& Texture::imageView() const
{
	return *mImageView;
}

uint32_t Texture::textureID() const
{
	return mID;
}

void Texture::createTextureImage(void* textureData, 
	uint32_t width, 
	uint32_t height, 
	VkDeviceSize imageSize,
	VkFormat imageFormat)
{
	// Create staging buffer to hold loaded data, ready to copy to device
	Buffer imageStagingBuffer(mDevice,
		imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Copy image data to staging buffer
	void* data = imageStagingBuffer.map();;
	memcpy(data, textureData, static_cast<size_t>(imageSize));
	imageStagingBuffer.unmap();

	// Check mipmap support and calculate number of mip levels
	bool mipmapSupport = checkMipmapGenerationSupport(imageFormat);

	uint32_t mipLevels;
	if (mipmapSupport)
	{
		// Calculate required number of mip levels using formula from OpenGL spec Section 8.14.3
		mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height))) + 1);
	}
	else
	{
		// No support for mipmaps so default is just the base texture (1 level)
		mipLevels = 1;
	}
	
	// Create image to hold base mipmap texture
	VkExtent2D newExtent = { width, height };
	mImage = std::make_unique<Image>(mDevice,
		newExtent,
		imageFormat,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT |	// DST since data will be copied to the image
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT |	// SRC since the image will be used as a source for blit operations
		VK_IMAGE_USAGE_SAMPLED_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_SAMPLE_COUNT_1_BIT,
		mipLevels);

	
	// COPY DATA TO IMAGE
	// Transition image to DST for copy operation (setup image memory barriers)
	std::unique_ptr<CommandBuffer> commandBuffer = mDevice.createAndBeginTemporaryCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	commandBuffer->transitionImageLayout(mImage->handle(), 
		VK_IMAGE_LAYOUT_UNDEFINED, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// Copy image data
	commandBuffer->copyBufferToImage(imageStagingBuffer, *mImage);

	// Transition image to be TRANSFER_SRC so that mip blit operations can read from it
	commandBuffer->transitionImageLayout(mImage->handle(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	// Generate the mipmaps if supported
	if (mipmapSupport)
	{
		generateMipmaps(mipLevels, width, height, *commandBuffer);
	}

	// Mipmap levels are now in VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL layout
	// They must be transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	commandBuffer->transitionImageLayout(mImage->handle(),
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		0,
		mipLevels);

	mDevice.endAndSubmitTemporaryCommandBuffer(*commandBuffer);

}
// Check physical device supports BLIT_SRC and BLIT_DST features for the image format
bool Texture::checkMipmapGenerationSupport(VkFormat format)
{
	VkFormatProperties formatProperties;
	mDevice.physicalDevice().getFormatProperties(format, formatProperties);

	// Check blit optimalTilingFeatures are supported
	if ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)
		&& (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT))
	{
		return true;
	}
	else
	{
		return false;
	}
}
// Generate mipmaps through blit operations
void Texture::generateMipmaps(uint32_t mipLevels, uint32_t width, uint32_t height, CommandBuffer& commandBuffer)
{

	// For each level copy mipmap i-1 to mipmap i
	for (uint32_t i = 1; i < mipLevels; ++i)
	{
		VkImageBlit imageBlit = {};

		int32_t srcX = std::max(static_cast<int32_t>(width >> (i - 1)), 1);
		int32_t srcY = std::max(static_cast<int32_t>(height >> (i - 1)), 1);

		int32_t dstX = std::max(static_cast<int32_t>(width >> i), 1);
		int32_t dstY = std::max(static_cast<int32_t>(height >> i), 1);

		// Source
		imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.srcSubresource.layerCount = 1;
		imageBlit.srcSubresource.mipLevel = i - 1;
		imageBlit.srcOffsets[1].x = srcX;		// Provide the mipmap dimensions by shifting the base image's dimensions
		imageBlit.srcOffsets[1].y = srcY;
		imageBlit.srcOffsets[1].z = 1;

		// Destination
		imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.dstSubresource.layerCount = 1;
		imageBlit.dstSubresource.mipLevel = i;
		imageBlit.dstOffsets[1].x = dstX;		// dst is shifted by 1 more so should be half the size of the src
		imageBlit.dstOffsets[1].y = dstY;
		imageBlit.dstOffsets[1].z = 1;

		// Transition current mip level to TRANSFER_DST to prepare as the blit operation destination
		commandBuffer.transitionImageLayout(mImage->handle(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			i);

		// Blit from mipmap level i-1 to i
		commandBuffer.blitImage(*mImage,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			*mImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			imageBlit);


		// Transition current mip level to SRC layout to prepare for blit of next mip level
		commandBuffer.transitionImageLayout(mImage->handle(),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			i);
	}


}

