#include "Camera.h"



Camera::Camera(mat4 viewMatrix, float FoVinDegrees, float width, float height, float zNear, float zFar) :
	mViewMatrix(viewMatrix), 
	mFoV(glm::radians(FoVinDegrees)), mWidth(width), mHeight(height), mZNear(zNear), mZFar(zFar),
	mIsOrtho(false)
{
	if (FoVinDegrees > 0.0f)
	{
		generatePerspective();
	}
	else
	{
		Camera(viewMatrix,  width, height, zNear, zFar);
	}
}

Camera::Camera(mat4 viewMatrix, float width, float height, float zNear, float zFar) :
	mViewMatrix(viewMatrix),
	mFoV(0.0f), mWidth(width), mHeight(height), mZNear(zNear), mZFar(zFar),
	mIsOrtho(false)
{
	generateOrtho();
}

mat4 Camera::getViewMatrix() const
{
	return mViewMatrix;
}

mat4 Camera::getProjectionMatrix() const
{
	return mProjectionMatrix;
}

float Camera::getWidth() const
{
	return mWidth;
}

float Camera::getHeight() const
{
	return mHeight;
}

float Camera::getZNear() const
{
	return mZNear;
}

float Camera::getZFar() const
{
	return mZFar;
}

float Camera::getFoV() const
{
	return mFoV;
}



bool Camera::isOrthographic() const
{
	return mIsOrtho;
}

void Camera::setViewMatrix(mat4 viewMatrix)
{
	mViewMatrix = viewMatrix;
}

void Camera::setProjectionMatrix(mat4 projectionMatrix, bool isOrthographic)
{
	mProjectionMatrix = projectionMatrix;
	mIsOrtho = isOrthographic;
}

void Camera::setPerspectiveProjection(float FoVinDegrees, float width, float height, float zNear, float zFar)
{
	mFoV = radians(FoVinDegrees);
	mWidth = width;
	mHeight = height;
	mZNear = zNear;
	mZFar = zFar;

	generatePerspective();
}

void Camera::setOrthoProjection(float width, float height, float zNear, float zFar)
{
	mWidth = width;
	mHeight = height;
	mZNear = zNear;
	mZFar = zFar;

	generateOrtho();
}

void Camera::setDimensions(float width, float height)
{
	mWidth = width;
	mHeight = height;

	float aspect = width / height;
	float tanHalfFoV = tan(mFoV / 2);
	mProjectionMatrix[0][0] = 1 / (aspect * tanHalfFoV);
}

void Camera::setClipping(float zNear, float zFar)
{
	mZNear = zNear;
	mZFar = zFar;

	mProjectionMatrix[2][2] = -(zFar + zNear) / (zFar - zNear);
	mProjectionMatrix[3][2] = -(2 * zFar * zNear) / (zFar - zNear);
}

void Camera::setFoV(float FoVinDegrees)
{
	mFoV = radians(FoVinDegrees);

	float aspect = mWidth / mHeight;
	float tanHalfFoV = tan(mFoV / 2);
	mProjectionMatrix[0][0] = 1 / (aspect * tanHalfFoV);
	mProjectionMatrix[1][1] = 1 / (tanHalfFoV);
}

void Camera::translate(vec3 position)
{
	glm::translate(mProjectionMatrix, position);
}

void Camera::rotate(float angleInDegrees, vec3 axis)
{
	glm::rotate(mProjectionMatrix, glm::radians(angleInDegrees), axis);
}

void Camera::generatePerspective()
{
	mIsOrtho = false;

	mProjectionMatrix = glm::perspective(mFoV, mWidth / mHeight, mZNear, mZFar);
}

void Camera::generateOrtho()
{
	mIsOrtho = true;

	float halfWidth = mWidth / 2;
	float halfHeight = mHeight / 2;

	mProjectionMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, mZNear, mZFar);
}


