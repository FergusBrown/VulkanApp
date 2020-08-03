#pragma once

#include "Light.h"

class DirectionalLight : public Light
{
public:
	DirectionalLight(const mat4& m, const vec3& c, float i) :
		Light(m, c, i), mDirection(vec3(0.0f, 0.0f, -1.0f))	// Default point forward
	{};

	DirectionalLight(const mat4& m, const vec3& c, float i, const vec3& dir) :
		Light(m, c, i), mDirection(glm::normalize(dir))
	{};

	const vec3& getDirection() { return mDirection; } const
		void setDirection(const vec3& dir) { this->mDirection = dir; }

private:
	vec3 mDirection;
};