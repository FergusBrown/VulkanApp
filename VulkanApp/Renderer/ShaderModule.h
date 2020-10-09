#pragma once
#include "Common.h"

class Device;

class ShaderModule
{
public:
	ShaderModule(Device& device, const std::vector<char>& shaderCode);
	~ShaderModule();

	VkShaderModule handle() const;

	ShaderModule(const ShaderModule&) = delete;
private:
	Device& mDevice;

	VkShaderModule mHandle{ VK_NULL_HANDLE };
};

