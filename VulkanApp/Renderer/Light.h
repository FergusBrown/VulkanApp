#pragma once
#include "Common.h"

// Note:
// https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/html/vkspec.html#interfaces-resources-layout
// - A scalar of size N has a base alignment of N
// - A three- or four-component vector, with components of size N, has a base alignment of 4 N
// *** IF USING VEC3 : ENSURE alignas(16) IS USED ***
// *** OTHERWISE USE VEC4 WHERE POSSIBLE ***
// *** STRUCTS MUST BE ALIGNED TO A MULTIPLE OF 16 ***


// Definition for simple light which is given a position and emits lights in all directions
// Fragment attenuation factor be calculated as (KqD^2 + KlD + Kc)^-1
// Where D is distance and the other constants are defined below
struct PointLight
{
	alignas(16) glm::vec4 colour{ glm::vec4(1.0f) };
	alignas(16) glm::vec4 position{ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) };		// Set w to 1 for position and 0 for direction
	alignas(4) float intensity{ 1.0f };
	alignas(4) float kq;
	alignas(4) float kl;
	alignas(4) float kc{ 1.0f };
};


// A light which in a direction in a limited angle
// the angle from the direction vector within which light is cast is defined by the cutoff
struct SpotLight
{
	alignas(16) glm::vec4 colour{ glm::vec4(1.0f) };
	alignas(16) glm::vec4 position{ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) };		// Set w to 1 for position and 0 for direction
	alignas(16) glm::vec4 direction{ glm::vec4(1.0f) };
	alignas(4) float intensity{ 1.0f };
	alignas(4) float cutOff; // This should be the cosine of the angle wanted as the cutoff
};

