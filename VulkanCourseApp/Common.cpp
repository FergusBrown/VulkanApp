#include "Common.h"

bool isDepthStencilFormat(VkFormat format)
{
	return	format == VK_FORMAT_D16_UNORM_S8_UINT ||
		format == VK_FORMAT_D24_UNORM_S8_UINT ||
		format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
		isDepthOnlyFormat(format);
}

bool isDepthOnlyFormat(VkFormat format)
{
	return	format == VK_FORMAT_D16_UNORM ||
		format == VK_FORMAT_D32_SFLOAT;
}