#include "DescriptorResourceReference.h"

#include "Buffer.h"
#include "Image.h"
#include "ImageView.h"
#include "Sampler.h"

void DescriptorResourceReference::reset()
{
	mResourceBindings.clear();
}

void DescriptorResourceReference::generateDescriptorImageInfo(VkDescriptorImageInfo& imageInfo, uint32_t bindingIndex, uint32_t arrayIndex)
{
	ResourceBinding& resource = mResourceBindings.at(bindingIndex).at(arrayIndex);

	if (resource.imageView == nullptr)
	{
		throw std::runtime_error("Attempting to create descriptor info for a resource which is not an image!");
	}

	imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = resource.imageView->handle();

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

// Generate infos for all bound resources
void DescriptorResourceReference::generateDescriptorInfos(BindingMap<VkDescriptorImageInfo>& imageInfos, BindingMap<VkDescriptorBufferInfo>& bufferInfos)
{
	for (auto& binding : mResourceBindings)
	{
		uint32_t bindingIndex = binding.first;
		auto& bindingContents = binding.second;

		for (auto& descriptor : bindingContents)
		{
			uint32_t descriptorIndex = descriptor.first;
			auto& resource = descriptor.second;

			if (resource.buffer)
			{
				VkDescriptorBufferInfo bufferInfo;
				generateDescriptorBufferInfo(bufferInfo, bindingIndex, descriptorIndex);
				bufferInfos[bindingIndex][descriptorIndex] = std::move(bufferInfo);
			}
			else // Otherwise is image
			{
				VkDescriptorImageInfo imageInfo;
				generateDescriptorImageInfo(imageInfo, bindingIndex, descriptorIndex);
				imageInfos[bindingIndex][descriptorIndex] = std::move(imageInfo);
			}
		}
	}
}


const BindingMap<ResourceBinding>& DescriptorResourceReference::resourceBindings() const
{
	return mResourceBindings;
}

// Image which can be sampled
void DescriptorResourceReference::bindImage(const ImageView& imageView, const Sampler& sampler, const uint32_t bindingIndex, const uint32_t arrayIndex)
{
	mResourceBindings[bindingIndex][arrayIndex].buffer =	nullptr;
	mResourceBindings[bindingIndex][arrayIndex].imageView =	&imageView;
	mResourceBindings[bindingIndex][arrayIndex].sampler =	&sampler;
}

// Input image (renderpass attachment)
void DescriptorResourceReference::bindInputImage(const ImageView& imageView, const uint32_t bindingIndex, const uint32_t arrayIndex)
{
	mResourceBindings[bindingIndex][arrayIndex].buffer =	nullptr;
	mResourceBindings[bindingIndex][arrayIndex].imageView =	&imageView;
	mResourceBindings[bindingIndex][arrayIndex].sampler =	nullptr;
}

void DescriptorResourceReference::bindBuffer(const Buffer& buffer, const uint32_t offset, const uint32_t range, const uint32_t bindingIndex, const uint32_t arrayIndex)
{
	mResourceBindings[bindingIndex][arrayIndex].buffer =	&buffer;
	mResourceBindings[bindingIndex][arrayIndex].offset =	offset;
	mResourceBindings[bindingIndex][arrayIndex].range =		range;
	mResourceBindings[bindingIndex][arrayIndex].imageView =	nullptr;
	mResourceBindings[bindingIndex][arrayIndex].sampler =	nullptr;
}
