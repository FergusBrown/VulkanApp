#pragma once

#include "Common.h"

#include "MeshModelData.h"

class MeshModel
{
public:
	MeshModel();
	MeshModel(int dataID);

	int getMeshDataID() const;

	glm::mat4 getModel() const;
	void setModel(glm::mat4 newModel);

private:
	int mMeshDataID;
	glm::mat4 mModel;

};