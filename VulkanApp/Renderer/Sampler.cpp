#include "Sampler.h"

#include "Device.h"

Sampler::Sampler(Device& device, 
	VkBool32 anisotropyEnable,
	float maxAnisotropy,
	float minLod,
	float maxLod,
	float mipLodBias,
	VkFilter magFilter, 
	VkFilter minFilter, 
	VkSamplerMipmapMode mipmapMode, 
	VkSamplerAddressMode addressModeU, 
	VkSamplerAddressMode addressModeV, 
	VkSamplerAddressMode addressModeW,
	VkBorderColor borderColor, 
	VkBool32 unnormalisedCoordinates) :
    mDevice(device)
{
	// Sampler creation Info
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = magFilter;								// How to render when image is magnified on screen
	samplerCreateInfo.minFilter = minFilter;								// How to render when image is minified on screen
	samplerCreateInfo.addressModeU = addressModeU;							// How to handle texture wrap in U (x) direction
	samplerCreateInfo.addressModeV = addressModeV;							// How to handle texture wrap in V (y) direction
	samplerCreateInfo.addressModeW = addressModeW;							// How to handle texture wrap in W (z) direction
	samplerCreateInfo.borderColor = borderColor;							// Border beyond texture (only works for border clamp)
	samplerCreateInfo.unnormalizedCoordinates = unnormalisedCoordinates;	// Whether coords should be normalised between 0 and 1
	samplerCreateInfo.mipmapMode = mipmapMode;								// Mipmap interpolation mode
	samplerCreateInfo.mipLodBias = mipLodBias;								// Level of details bias for mipmap level
	samplerCreateInfo.maxLod = maxLod;										// Maximum level of detail to pick mip level
	samplerCreateInfo.minLod = minLod;										// Minimum level of detail to pick mip level
	samplerCreateInfo.anisotropyEnable = anisotropyEnable;					// Enable Anisotropy
	samplerCreateInfo.maxAnisotropy = maxAnisotropy;						// Anistropy sample level

	VkResult result = vkCreateSampler(mDevice.logicalDevice(), &samplerCreateInfo, nullptr, &mHandle);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Texture Sampler!");
	}
}

Sampler::~Sampler()
{
    vkDestroySampler(mDevice.logicalDevice(), mHandle, nullptr);
}

VkSampler Sampler::handle() const
{
    return mHandle;
}
