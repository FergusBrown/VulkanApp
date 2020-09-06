#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <algorithm>
#include <array>
#include <cstring>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <set>
#include <map>				// Ordered map
#include <unordered_map>	// Hash map
#include <vector>

/// *** BindingMap ***
/// Used to index descriptors and map them to their binding
/// Top layer key maps to the descriptor set binding
/// Bottom layer key maps to a descriptor within the array of descriptors in the binding
template <typename T>
using BindingMap = std::map<uint32_t, std::map<uint32_t, T>>;