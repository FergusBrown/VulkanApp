#pragma once
#include "Common.h"

// Definition for simple light which is given a position and emits lights in all directions
struct Light
{
	glm::vec3 colour{ glm::vec3(1.0f) };
	float intensity{1.0f};
	glm::vec3 position;		// Set w to 1 for position and 0 for direction
};

// Point light is defined by attenuation constants
// Fragment attenuation factor be calculated as (KqD^2 + KlD + Kc)^-1
// Where D is distance and the other constants are defined below
struct PointLight : Light
{
	float Kq;
	float Kl;
	float Kc;
};

// A light which in a direction in a limited angle
// the angle from the direction vector within which light is cast is defined by the cutoff
struct SpotLight : Light
{
	glm::vec3 direction;
	float cutOff;
};