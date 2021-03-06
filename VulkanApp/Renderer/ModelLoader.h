#pragma once
#include "Common.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Device;
class Mesh;
struct Vertex;

void LoadMaterials(const aiScene* scene, 
	std::map<uint32_t, 
	std::string>& diffuseList, 
	std::map<uint32_t, 
	std::string>& normalList, 
	std::map<uint32_t, std::string>& specularList, 
	std::map<uint32_t, bool>& isMaterialOpaque);
std::vector<std::unique_ptr<Mesh>> LoadNode(Device& device, aiNode* node, const aiScene* scene, std::vector<uint32_t>& materialIDs, std::map<uint32_t, bool>& isMaterialOpaque);

std::unique_ptr<Mesh> LoadMesh(Device& device, aiMesh* mesh, const aiScene* scene, std::vector<uint32_t> materialIDs, std::map<uint32_t, bool>& isMaterialOpaque);

void calculateTangentBasis(std::vector<Vertex>& vertices);

void reOrthogonalise(std::vector<Vertex>& vertices);