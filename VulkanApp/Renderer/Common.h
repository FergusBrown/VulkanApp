#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <algorithm>
#include <array>
#include <cstring>
#include <cmath>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <set>
#include <map>				
#include <unordered_map>	
#include <vector>

// *** Max Value Constants ***
const uint32_t MAX_TEXTURES = 2048;
const float MAX_LOD		= 15.0f;	// This should support all mip levels for textures of resolution up to 16K resolution
const uint32_t MAX_OBJECTS	= 10;


/// *** BindingMap ***
/// Used to index descriptors and map them to their binding
/// Top layer key maps to the descriptor set binding
/// Bottom layer key maps to a descriptor within the array of descriptors in the binding
template <typename T>
using BindingMap = std::map<uint32_t, std::map<uint32_t, T>>;

// Check to see if format is a depth stencil format
// See "_D" and "_S" formats in following documentation:
// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#formats-compatibility
bool isDepthStencilFormat(VkFormat format);

bool isDepthOnlyFormat(VkFormat format);


