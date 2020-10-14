#pragma once
#include "Common.h"

class CommandBuffer;
class Device;
class Image;
class ImageView;

class Texture
{
public:
	Texture(Device& device, 
		void* textureData, 
		int width, 
		int height, 
		VkDeviceSize imageSize,
		VkFormat imageFormat = VK_FORMAT_R8G8B8A8_UNORM);
	~Texture() = default;

	// - Getters
	const Device& device() const;
	const ImageView& imageView() const;
	uint32_t textureID() const;

private:
	Device& mDevice;

	static uint32_t ID;
	uint32_t mID;

	std::unique_ptr<Image> mImage;
	std::unique_ptr<ImageView> mImageView;
	
	// - Texture Creation
	void createTextureImage(void* textureData, 
		uint32_t width, 
		uint32_t height, 
		VkDeviceSize imageSize,
		VkFormat imageFormat);
	// -- Support
	bool checkMipmapGenerationSupport(VkFormat format);
	void generateMipmaps(uint32_t mipLevels, uint32_t width, uint32_t height, CommandBuffer& commandBuffer);
};

