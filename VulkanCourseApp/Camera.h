#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

class Camera
{
public:
	// Perspective Projection
	Camera(mat4 viewMatrix, float FoVinDegrees, float width, float height, float zNear, float zFar);

	// Orthographic Projection
	Camera(mat4 viewMatrix, float width, float height, float zNear, float zFar);

	// Getters
	mat4 getViewMatrix() const;
	mat4 getProjectionMatrix() const;

	float getWidth() const;
	float getHeight() const;
	float getZNear() const;
	float getZFar() const;
	float getFoV() const;
	bool isOrthographic() const;

	// Setters
	void setViewMatrix(mat4 viewMatrix);
	void setProjectionMatrix(mat4 projectionMatrix, bool isOrthographic);

	void setPerspectiveProjection(float FoVinDegrees, float width, float height, float zNear, float zFar);
	void setOrthoProjection(float width, float height, float zNear, float zFar);

	void setDimensions(float width, float height);
	void setClipping(float zNear, float zFar);
	void setFoV(float FoVinDegrees);

	// View manipulation
	void translate(vec3 position);
	void rotate(float angleInDegrees, vec3 axis);
private:

	// Matrices
	mat4 mViewMatrix;
	mat4 mProjectionMatrix;

	// Dimensions / Aspect
	float mHeight;
	float mWidth;

	// Clipping Planes
	float mZNear;
	float mZFar;

	// Perspective values
	float mFoV;

	bool mIsOrtho;

	// Helper
	void generatePerspective();
	void generateOrtho();
};

