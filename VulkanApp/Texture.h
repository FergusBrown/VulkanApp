#pragma once
#include "Common.h"

class Device;
class Image;
class ImageView;

class Texture
{
public:
	Texture(Device& device, void* textureData, int width, int height, VkDeviceSize imageSize);
	~Texture() = default;

	// - Getters
	const Device& device() const;
	//const Image& image() const;
	//const VkImage& image() const;
	const ImageView& imageView() const;
	//VkDeviceMemory memory() const;
	uint32_t textureID() const;

	//static void createTextureImage(std::string fileName);
	//static stbi_uc* loadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize);

private:
	Device& mDevice;

	static uint32_t ID;
	uint32_t mID;

	std::unique_ptr<Image> mImage;
	std::unique_ptr<ImageView> mImageView;

	void createTextureImage(void* textureData, uint32_t width, uint32_t height, VkDeviceSize imageSize);
};

