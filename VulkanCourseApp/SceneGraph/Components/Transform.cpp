#include "Transform.h"

#include <glm/gtx/matrix_decompose.hpp>

Transform::Transform(Node& node) :
	mNode(node)
{
}

Node& Transform::node()
{
	return mNode;
}

std::type_index Transform::type()
{
	return typeid(Transform);
}

const glm::vec3& Transform::translation() const
{
	return mTranslation;
}

const glm::quat& Transform::rotation() const
{
	return mRotation;
}

const glm::vec3& Transform::scale() const
{
	return mScale;
}

glm::mat4 Transform::matrix() const
{
	return	glm::translate(glm::mat4(1.0), mTranslation) *
		glm::mat4_cast(mRotation) *
		glm::scale(glm::mat4(1.0), mScale);
}

void Transform::setTranslation(const glm::vec3& translation)
{
	mTranslation = translation;
}

void Transform::setRotation(const glm::quat& rotation)
{
	mRotation = rotation;
}

void Transform::setScale(const glm::vec3& scale)
{
	mScale = scale;
}

void Transform::setMatrix(const glm::mat4& modelMatrix)
{
	glm::vec3 skew;
	glm::vec4 perspective;

	glm::decompose(modelMatrix, mScale, mRotation, mTranslation, skew, perspective);
	mRotation = glm::conjugate(mRotation);
}
