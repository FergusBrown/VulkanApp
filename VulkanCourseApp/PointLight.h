#pragma once

#include "Light.h"

class PointLight : public Light
{
public:
	PointLight(const mat4& m, const vec3& c, float i) :
		Light(m, c, i), mPosition(vec3(0.0f))
	{};

	PointLight(const mat4& m, const vec3& c, float i, const vec3& pos) :
		Light(m, c, i),  mPosition(pos)
	{};

	const vec3& getPosition() { return mPosition; } const
	void setPosition(const vec3& pos) { this->mPosition = pos; }

private:
	vec3 mPosition;
};