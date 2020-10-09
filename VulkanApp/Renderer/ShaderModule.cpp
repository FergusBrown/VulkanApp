#include "ShaderModule.h"

#include "Device.h"

ShaderModule::ShaderModule(Device& device, const std::vector<char>& shaderCode) :
	mDevice(device)
{
	// Shader module creation info
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = shaderCode.size();										// Size of code
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());		// Pointer to code (of uint32_t pointer type)

	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(mDevice.logicalDevice(), &shaderModuleCreateInfo, nullptr, &shaderModule);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Shader Module!");
	}
}

ShaderModule::~ShaderModule()
{
	if (mHandle != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(mDevice.logicalDevice(), mHandle , nullptr);
	}
}

VkShaderModule ShaderModule::handle() const
{
	return mHandle;
}
