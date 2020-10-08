#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>

using namespace glm;

class Light
{
public:
	Light(const mat4& m, const vec3& c, float i) : mModel(m), mColour(c), mIntensity(i) {}
	virtual ~Light() {}

private:
	mat4 mModel;
	vec3 mColour;
	float mIntensity;
};

