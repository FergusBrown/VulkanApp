#include "DescriptorResourceSet.h"

void DescriptorResourceSet::reset()
{
	mResourceBindings.clear();
}

void DescriptorResourceSet::generateDescriptorImageInfo(VkDescriptorImageInfo& imageInfo, uint32_t bindingIndex, uint32_t arrayIndex)
{
	Resource& resource = mResourceBindings.at(bindingIndex).at(arrayIndex);

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
		imageInfo.sampler = Sampler->handle();
	}

}

const BindingMap<Resource>& DescriptorResourceSet::resourceBindings() const
{
	return mResourceBindings;
}

void DescriptorResourceSet::bindInputImage(const Image& image, const uint32_t bindingIndex, const uint32_t arrayIndex)
{
	mResourceBindings[bindingIndex][arrayIndex].buffer =	nullptr;
	mResourceBindings[bindingIndex][arrayIndex].image =		&image;
	mResourceBindings[bindingIndex][arrayIndex].sampler =	nullptr;
}
