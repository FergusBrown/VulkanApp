#pragma once
#include "Common.h"

#include "Device.h"
#include "Image.h"
#include "Buffer.h"

class Texture
{
public:
	Texture(Device& device, stbi_uc* textureData, int width, int height, VkDeviceSize imageSize);
	~Texture() = default;

	// - Getters
	const Device& device() const;
	const VkImage& image() const;
	const VkImageView& imageView() const;
	VkDeviceMemory memory() const;
	uint32_t textureID() const;

	//static void createTextureImage(std::string fileName);
	static stbi_uc* loadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize);

private:
	Device& mDevice;

	static uint32_t ID;
	uint32_t mID;

	std::unique_ptr<Image> mImage;

	void createImage(stbi_uc* textureData, int width, int height, VkDeviceSize imageSize);
};
uint32_t Texture::ID = 0;
