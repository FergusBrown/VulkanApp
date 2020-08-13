#pragma once

#include <vector>;

#include <glm/glm.hpp>

#include "MeshModelData.h"

class MeshModel
{
public:
	MeshModel(MeshModelData* meshModelData);

	size_t getMeshCount();
	Mesh* getMesh(size_t index);

	glm::mat4 getModel();
	void setModel(glm::mat4 newModel);

private:
	MeshModelData* mMeshData;
	glm::mat4 mModel;

};