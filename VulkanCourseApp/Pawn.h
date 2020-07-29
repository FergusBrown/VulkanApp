#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

class Pawn
{
public:
	Pawn();

	const mat4& generateView() const;

	void translate(vec3 position);
	void rotate(vec3 eulerAngles);

private:
	vec3 mPosition;

	float mXRotation;
	float mYRotation;
	float mZRotation;
	
	// Helper
	void fixExcessRotation(float& angle);

};

