#include "MeshModel.h"

MeshModel::MeshModel(MeshModelData* meshModelData) : 
	mMeshData(meshModelData), mModel(glm::mat4(1.0f))
{
}

size_t MeshModel::getMeshCount()
{
	return mMeshData->getMeshCount();
}

Mesh* MeshModel::getMesh(size_t index)
{
	return mMeshData->getMesh(index);
}

glm::mat4 MeshModel::getModel()
{
	return mModel;
}

void MeshModel::setModel(glm::mat4 newModel)
{
	mModel = newModel;
}
