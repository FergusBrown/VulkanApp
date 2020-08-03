#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

class Pawn
{
public:
	Pawn();

	const mat4& generateView() const;

	void movePositionBy(vec3 position);
	void moveForwardBy(float distance);
	void moveRightBy(float distance);
	void moveUpBy(float distance);
	void rotate(vec3 eulerAngles);

private:

	// Position of camera
	vec3 mPosition;

	// look at direction and associated vectors
	vec3 mDirection;
	vec3 mRight;
	vec3 mUp;

	float mXRotation;
	float mYRotation;
	float mZRotation;
	
	// Helper
	void fixExcessRotation(float& angle);
	void updateLookValues();
};

