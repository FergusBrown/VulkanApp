#pragma once
#include "Common.h"

#include "Image.h"
#include "Sampler.h"

// Struct contating pointer to the appropriate resource
struct Resource
{
	const Buffer*	buffer{ nullptr };
	uint32_t		offset{ 0 };
	uint32_t		range{ 0 };
	const Image*	image{ nullptr };
	const Sampler*	sampler{ nullptr };
};

// Contains actual resources references in descriptor bindings
// + functions for generating descriptor infos
class DescriptorResourceSet
{
public:
	DescriptorResourceSet() = default;
	~DescriptorResourceSet() = default;

	// - Getters

	// - Management
	void reset();
	void generateDescriptorImageInfo(VkDescriptorImageInfo& imageInfo, uint32_t bindingIndex, uint32_t arrayIndex);
	void getDescriptorBufferInfo(VkDescriptorBufferInfo& bufferInfo, uint32_t bindingIndex, uint32_t arrayIndex);

	const BindingMap<Resource>& resourceBindings() const;
	void bindBuffer(const Buffer& buffer, const uint32_t offset, const uint32_t range, const uint32_t bindingIndex, const uint32_t arrayIndex);
	void bindImage(const Image& image, const Sampler& sampler, const uint32_t bindingIndex, const uint32_t arrayIndex);
	void bindInputImage(const Image& image, const uint32_t bindingIndex, const uint32_t arrayIndex);

private:
	BindingMap<Resource> mResourceBindings;

};
