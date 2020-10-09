#pragma once

#include "Common.h"

class Mesh;

class MeshModel
{
public:
	MeshModel() = default;
	MeshModel(std::vector<std::unique_ptr<Mesh>>& meshList);

	size_t meshCount() const;
	Mesh& mesh(size_t index);
	glm::mat4 modelMatrix() const;
	void setModel(glm::mat4& newModel);

private:
	std::vector<std::unique_ptr<Mesh>> mMeshList;
	glm::mat4 mModel;

};