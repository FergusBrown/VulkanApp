#pragma once
#include "Common.h"

class Device;

class ShaderModule
{
public:
	ShaderModule(Device& device, 
		const std::vector<char>& shaderCode,
		VkShaderStageFlagBits stageFlagBits);
	~ShaderModule();

	ShaderModule(const ShaderModule&) = delete;

	ShaderModule(ShaderModule&& other) noexcept;

	VkShaderModule handle() const;

	VkShaderStageFlagBits stageFlagBits() const;

	
private:
	Device& mDevice;

	VkShaderModule mHandle{ VK_NULL_HANDLE };

	VkShaderStageFlagBits mStageFlagBits;
};

