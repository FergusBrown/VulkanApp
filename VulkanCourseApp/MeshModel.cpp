#include "MeshModel.h"

//#include "MeshModelData.h"


MeshModel::MeshModel(int dataID) :
	mMeshDataID(dataID), mModel(glm::mat4(1.0f))
{
}

int MeshModel::getMeshDataID() const
{
	return mMeshDataID;
}

glm::mat4 MeshModel::getModel() const
{
	return mModel;
}

void MeshModel::setModel(glm::mat4 newModel)
{
	mModel = newModel;
}
