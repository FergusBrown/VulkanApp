#pragma once
#include "Common.h"

// Definitions for simple delta lights
struct Light
{
	//glm::mat4 worldTransform{ glm::mat4(1.0f) };
	glm::vec3 colour{ glm::vec3(1.0f) };
	float intensity{1.0f};
};

struct PointLight : Light
{
	glm::vec3 position{glm::vec3(0.0f)};
};

struct DirectionalLight : Light
{
	glm::vec3 direction{glm::vec3(0.0f)};
};