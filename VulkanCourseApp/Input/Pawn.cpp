#include "Pawn.h"

#define _USE_MATH_DEFINES
#include "math.h"

#include <algorithm>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


Pawn::Pawn()  :
	mXRotation(0.0f), mYRotation(M_PI), mZRotation(0.0f),
	mPosition(vec3(0.0f, 0.0f, 10.0f)), 
	mDirection(vec3(0.0f, 0.0f, -1.0f)), mRight(vec3(1.0f, 0.0f, 0.0f)), mUp(vec3(0.0f, 1.0f, 0.0f))
{
}

const mat4& Pawn::generateView() const
{
	
	return glm::lookAt(
		mPosition,           // Camera is here
		mPosition + mDirection, // and looks here : at the same position, plus "direction"
		mUp                  // Head is up (set to 0,-1,0 to look upside-down)
	);
}

void Pawn::movePositionBy(vec3 position)
{
	mPosition +=  position;
}

void Pawn::moveForwardBy(float distance)
{
	mPosition += mDirection * distance;
}

void Pawn::moveRightBy(float distance)
{
	mPosition += mRight * distance;
}

void Pawn::moveUpBy(float distance)
{
	mPosition += mUp * distance;
}

// Recreate view matrix on each rotation to avoid rounding errors
void Pawn::rotate(vec3 eulerAngles)
{
	// Compute new orientation
	mXRotation += eulerAngles.x;
	mYRotation += eulerAngles.y;
	mZRotation += eulerAngles.z;

	// Clamp vertical so that there is only a 180 degree range of vision vertically
	mXRotation = std::clamp(mXRotation, glm::radians(-90.0f), glm::radians(90.0f));
	fixExcessRotation(mYRotation);
	fixExcessRotation(mZRotation);

	updateLookValues();
}

void Pawn::fixExcessRotation(float& angle)
{
	angle = fmod(angle , (2 *  M_PI));

	if (angle < 0.0f)
	{
		angle += 2 * M_PI;
	}
		
}

void Pawn::updateLookValues()
{

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	mDirection = glm::normalize(vec3(
		cos(mZRotation) * sin(mYRotation) * cos(mXRotation) + sin(mZRotation) * sin(mXRotation),
		-sin(mZRotation) * sin(mYRotation) * cos(mXRotation) + cos(mZRotation) * sin(mXRotation),
		cos(mYRotation) * cos(mXRotation)
	));



	// Right vector - can only be rotated around Z and Y axis
	mRight = glm::normalize(vec3(
		sin(mYRotation - M_PI / 2.0f) * cos(mZRotation),
		sin(mYRotation - M_PI / 2.0f) * sin(mZRotation),
		cos(mYRotation - M_PI / 2.0f)
	));


	// Up vector
	mUp = glm::cross(mRight, mDirection);
}
