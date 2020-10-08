#pragma once
#include <assimp/scene.h>

#include "Common.h"

class Device;
class Mesh;



// Class for loading mesh and material data + default position
class MeshModelData
{
public:
	MeshModelData(std::vector<Mesh*> newMeshList);

	size_t getMeshCount();
	Mesh* getMesh(size_t index);

	/*glm::mat4 getModel();
	void setModel(glm::mat4 newModel);*/

	//void destroyMeshModel();

	static std::vector<std::string> LoadMaterials(const aiScene* scene);
	/*static std::vector<Mesh*> LoadNode(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool,
		aiNode* node, const aiScene* scene, std::vector<int> matToTex);*/
	static std::vector<Mesh*> LoadNode(Device& device, aiNode* node, const aiScene* scene, std::vector<int> matToTex);
	/*static Mesh* LoadMesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool,
		aiMesh* mesh, const aiScene* scene, std::vector<int> matToTex);*/
	static Mesh* LoadMesh(Device& device, aiMesh* mesh, const aiScene* scene, std::vector<int> matToTex);


private:
	std::vector<Mesh*> meshList;
	//glm::mat4 model;

};



