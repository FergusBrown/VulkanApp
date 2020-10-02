#pragma once
#include <string>
#include <typeinfo>
#include <vector>

#include "glmCommon.h"
#include <glm/gtx/quaternion.hpp>

#include "SceneGraph/Component.h"

class Node;

// Transformation matrix component for a scene graph node
class Transform : public Component
{
public:
	Transform(Node& node);
	~Transform() = default;

	// - Getters
	Node& node();

	virtual std::type_index type() override;

	const glm::vec3& translation() const;

	const glm::quat& rotation() const;

	const glm::vec3& scale() const;

	glm::mat4 matrix() const;

	// - Setters
	void setTranslation(const glm::vec3& translation);

	void setRotation(const glm::quat& rotation);

	void setScale(const glm::vec3& scale);

	void setMatrix(const glm::mat4& modelMatrix);

private:
	Node& mNode;

	glm::vec3 mTranslation = glm::vec3(0.0, 0.0, 0.0);

	glm::quat mRotation = glm::quat(1.0, 0.0, 0.0, 0.0);

	glm::vec3 mScale = glm::vec3(1.0, 1.0, 1.0);

	glm::mat4 mWorldMatrix = glm::mat4(1.0);

};

