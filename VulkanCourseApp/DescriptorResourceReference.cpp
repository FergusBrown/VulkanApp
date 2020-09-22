#include "DescriptorResourceReference.h"

#include "Buffer.h"
#include "Image.h"
#include "Sampler.h"

void DescriptorResourceReference::reset()
{
	mResourceBindings.clear();
}

void DescriptorResourceReference::generateDescriptorImageInfo(VkDescriptorImageInfo& imageInfo, uint32_t bindingIndex, uint32_t arrayIndex)
{
	ResourceBinding& resource = mResourceBindings.at(bindingIndex).at(arrayIndex);

	if (resource.image == nullptr)
	{
		throw std::runtime_error("Attempting to create descriptor info for a resource which is not an image!");
	}

	imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = resource.image->imageView();

	if (resource.sampler == nullptr)
	{
		imageInfo.sampler = VK_NULL_HANDLE;
	}
	else
	{
		imageInfo.sampler = resource.sampler->handle();
	}

}

void DescriptorResourceReference::generateDescriptorBufferInfo(VkDescriptorBufferInfo& bufferInfo, uint32_t bindingIndex, uint32_t arrayIndex)
{
	ResourceBinding& resource = mResourceBindings.at(bindingIndex).at(arrayIndex);

	if (resource.buffer == nullptr)
	{
		throw std::runtime_error("Attempting to create descriptor info for a resource which is not a buffer!");
	}

	bufferInfo = {};
	bufferInfo.buffer = resource.buffer->handle();		// Buffer to get data from
	bufferInfo.offset = resource.offset;				// Position of start of data
	bufferInfo.range =  resource.range;					// Size of data
}

const BindingMap<ResourceBinding>& DescriptorResourceReference::resourceBindings() const
{
	return mResourceBindings;
}

void DescriptorResourceReference::bindImage(const Image& image, const Sampler& sampler, const uint32_t bindingIndex, const uint32_t arrayIndex)
{
	mResourceBindings[bindingIndex][arrayIndex].buffer =	nullptr;
	mResourceBindings[bindingIndex][arrayIndex].image =		&image;
	mResourceBindings[bindingIndex][arrayIndex].sampler =	&sampler;
}

void DescriptorResourceReference::bindInputImage(const Image& image, const uint32_t bindingIndex, const uint32_t arrayIndex)
{
	mResourceBindings[bindingIndex][arrayIndex].buffer =	nullptr;
	mResourceBindings[bindingIndex][arrayIndex].image =		&image;
	mResourceBindings[bindingIndex][arrayIndex].sampler =	nullptr;
}

void DescriptorResourceReference::bindBuffer(const Buffer& buffer, const uint32_t offset, const uint32_t range, const uint32_t bindingIndex, const uint32_t arrayIndex)
{
	mResourceBindings[bindingIndex][arrayIndex].buffer =	nullptr;
	mResourceBindings[bindingIndex][arrayIndex].offset =	offset;
	mResourceBindings[bindingIndex][arrayIndex].range =		range;
	mResourceBindings[bindingIndex][arrayIndex].image =		nullptr;
	mResourceBindings[bindingIndex][arrayIndex].sampler =	nullptr;
}
