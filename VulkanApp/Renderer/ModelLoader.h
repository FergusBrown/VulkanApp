#pragma once
#include "Common.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Device;
class Mesh;

std::vector<std::string> LoadMaterials(const aiScene* scene);
std::vector<std::unique_ptr<Mesh>> LoadNode(Device& device, aiNode* node, const aiScene* scene, std::vector<int> matToTex);

std::unique_ptr<Mesh> LoadMesh(Device& device, aiMesh* mesh, const aiScene* scene, std::vector<int> matToTex);