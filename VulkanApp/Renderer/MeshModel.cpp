#include "MeshModel.h"

#include "Mesh.h"

MeshModel::MeshModel(std::vector<std::unique_ptr<Mesh>>& meshList) :
	mMeshList(std::move(meshList)), mModel(glm::mat4(1.0f))
{
}

size_t MeshModel::meshCount() const
{
	return mMeshList.size();
}

Mesh& MeshModel::mesh(size_t index)
{
	return *mMeshList.at(index);
}

glm::mat4 MeshModel::modelMatrix() const
{
	return mModel;
}

void MeshModel::setModel(glm::mat4& newModel)
{
	mModel = newModel;
}
