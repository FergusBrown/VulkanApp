#include "Pawn.h"

#define _USE_MATH_DEFINES
#include "math.h"

#include <algorithm>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


Pawn::Pawn()  :
	mXRotation(0.0f), mYRotation(M_PI), mZRotation(0.0f), mPosition(vec3(0.0f, 0.0f, 10.0f))
{
	//mView = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

const mat4& Pawn::generateView() const
{

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(mXRotation) * sin(mYRotation),
		sin(mXRotation),
		cos(mXRotation) * cos(mYRotation)
	);

	// Right vector
	glm::vec3 right = glm::vec3(
		sin(mYRotation - M_PI / 2.0f),
		0,
		cos(mYRotation - M_PI / 2.0f)
	);

	// Up vector
	glm::vec3 up = glm::cross(right, direction);

	return glm::lookAt(
		mPosition,           // Camera is here
		mPosition + direction, // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
	);
}

//const mat4& Pawn::getView() const
//{
//	return mView;
//}

void Pawn::translate(vec3 position)
{
	mPosition +=  position;
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

	//glm::quat bearingQuat(eulerAngles);

	// TODO: feels like there's a simpler way to do this rather than converting to quaternion
	// Is this faster than just calculating cartesian coords from euler angles?
	//mat4 rotationMatrix = glm::toMat4(bearingQuat);

	//mView = mView;
}

void Pawn::fixExcessRotation(float& angle)
{
	angle = fmod(angle , (2 *  M_PI));

	if (angle < 0.0f)
	{
		angle += 2 * M_PI;
	}
		
}
