#pragma once
#include "Common.h"

class Device;

class Sampler
{
public:
	Sampler(Device&				device,
		VkBool32				anisotropyEnable = VK_TRUE,
		float					maxAnisotropy = 0.0f,
		float					minLod = 0.0f,
		float					maxLod = 0.0f,
		float					mipLodBias = 0.0f,
		VkFilter				magFilter = VK_FILTER_LINEAR,
		VkFilter				minFilter = VK_FILTER_LINEAR,
		VkSamplerMipmapMode		mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		VkSamplerAddressMode    addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VkSamplerAddressMode    addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VkSamplerAddressMode    addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VkBorderColor			borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		VkBool32				unnormalisedCoordinates = VK_FALSE);
	~Sampler();

	// - Getters
	VkSampler handle() const;
private:
	Device& mDevice;

	VkSampler mHandle{ VK_NULL_HANDLE };

};

