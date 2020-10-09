//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>
#include "Texture.h"

#include "Buffer.h"
#include "CommandBuffer.h"
#include "Device.h"
#include "Image.h"
#include "ImageView.h"

uint32_t Texture::ID = 0;

Texture::Texture(Device& device, void* textureData, int width, int height, VkDeviceSize imageSize) :
	mDevice(device), mID(ID)
{
	createTextureImage(textureData, static_cast<uint32_t>(width), static_cast<uint32_t>(height), imageSize);

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
//
//// TODO : consider move this out of this class and back to vulkan renderer
//stbi_uc* Texture::loadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize)
//{
//	// Number of channels image uses
//	int channels;
//
//	// Load pixel data for image
//	std::string fileLoc = "Textures/" + fileName;
//	stbi_uc* image = stbi_load(fileLoc.c_str(), width, height, &channels, STBI_rgb_alpha);
//
//	if (!image)
//	{
//		throw std::runtime_error("Failed to load a Texture File! (" + fileName + ")");
//	}
//
//	// calculate image size using given and known data (note 4 is for RGB and A channels)
//	*imageSize = *width * *height * 4;
//
//	return image;
//}

void Texture::createTextureImage(void* textureData, uint32_t width, uint32_t height, VkDeviceSize imageSize)
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

	// Free original image data
	//stbi_image_free(textureData);

	// Create image to hold final texture
	VkExtent2D newExtent = { width, height };
	mImage = std::make_unique<Image>(mDevice,
		newExtent,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); 

	
	// COPY DATA TO IMAGE
	// Transition image to DST for copy operation (setup image memory barriers)
	std::unique_ptr<CommandBuffer> commandBuffer = mDevice.createAndBeginTemporaryCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	commandBuffer->transitionImageLayout(mImage->handle(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// Copy image data
	commandBuffer->copyBufferToImage(imageStagingBuffer, *mImage);

	// Transition image to be shader readable for shader usage
	commandBuffer->transitionImageLayout(mImage->handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	mDevice.endAndSubmitTemporaryCommandBuffer(*commandBuffer);

	// Add texture data to vector for reference
	//textureImages.push_back(texImage);
	//textureImageMemory.push_back(texImageMemory);

	// Destroy staging buffers
	//vkDestroyBuffer(mDevice->logicalDevice(), imageStagingBuffer, nullptr);
	//vkFreeMemory(mDevice->logicalDevice(), imageStagingBufferMemory, nullptr);

	// Return index of new texture image
	//return textureImages.size() - 1;
}

