#pragma once

#include "Common.h"

class MeshModel
{
public:
	MeshModel() = default;
	MeshModel(int dataID);

	int getMeshDataID() const;

	glm::mat4 getModel() const;
	void setModel(glm::mat4 newModel);

private:
	int mMeshDataID;
	glm::mat4 mModel;

};