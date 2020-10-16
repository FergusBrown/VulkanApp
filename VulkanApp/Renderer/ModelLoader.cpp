#include "ModelLoader.h"

#include "Mesh.h"

std::vector<std::string> LoadMaterials(const aiScene* scene)
{
	// Create 1:1 sized list of textures
	std::vector<std::string> textureList(scene->mNumMaterials);

	// Got through each material and copy its texture file name (if it exists)
	for (size_t i = 0; i < scene->mNumMaterials; ++i)
	{
		// Get the material
		aiMaterial* material = scene->mMaterials[i];

		// Initalise the texture to empty string (will be replaced if texture exists)
		textureList[i] = "";

		// Check for a Diffuse Texture (standard detail texture)
		if (material->GetTextureCount(aiTextureType_DIFFUSE))
		{
			aiString path;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
			{
				// Cut off any directory information already present
				int idx = std::string(path.data).rfind("\\");
				std::string fileName = std::string(path.data).substr(idx + 1);

				textureList[i] = fileName;
			}
		}

		// Check for a Normal Texture 
		//if (material->GetTextureCount(aiTextureType_NORMALS))
		//{
		//	aiString path;
		//	if (material->GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS)
		//	{
		//		// Cut off any directory information already present
		//		int idx = std::string(path.data).rfind("\\");
		//		std::string fileName = std::string(path.data).substr(idx + 1);

		//		textureList[i] = fileName;
		//	}
		//}
	}

	return textureList;
}

std::vector<std::unique_ptr<Mesh>> LoadNode(Device& device, aiNode* node, const aiScene* scene, std::vector<int> matToTex)
{
	std::vector<std::unique_ptr<Mesh>> meshList;

	// Go through mesh at this node and create it, then add it to out meshList;
	for (size_t i = 0; i < node->mNumMeshes; ++i)
	{
		std::unique_ptr<Mesh> meshPtr = LoadMesh(device, scene->mMeshes[node->mMeshes[i]], scene, matToTex);

		meshList.push_back(std::move(meshPtr));
	}

	// Go through each node attached to this node and load it, then append their meshes to this node's mesh list
	for (size_t i = 0; i < node->mNumChildren; ++i)
	{
		std::vector<std::unique_ptr<Mesh>> newList = LoadNode(device, node->mChildren[i], scene, matToTex);

		for (auto& meshPtr : newList)
		{
			meshList.push_back(std::move(meshPtr));
		}

		//meshList.insert(meshList.end(), newList.begin(), newList.end());
	}

	return meshList;
}

std::unique_ptr<Mesh> LoadMesh(Device& device, aiMesh* mesh, const aiScene* scene, std::vector<int> matToTex)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	// Resixe vertex list to hold all vertices for mesh
	vertices.resize(mesh->mNumVertices);

	// Go through each vertex and copy it across to our vertices
	for (size_t i = 0; i < mesh->mNumVertices; ++i)
	{
		// Set position
		vertices[i].position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };

		// Set normal
		vertices[i].normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

		// Set tex coords (if they exist)
		if (mesh->mTextureCoords[0])
		{
			vertices[i].uv = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
		}
		else
		{
			vertices[i].uv = { 0.0f, 0.0f };
		}

		// Set colour
		//vertices[i].col = { 1.0f, 1.0f, 1.0f };
	}

	// Iterate over indices through faces and copy across
	for (size_t i = 0; i < mesh->mNumFaces; ++i)
	{
		// Get a face
		aiFace face = mesh->mFaces[i];

		// For through face's indices and add to list
		for (size_t j = 0; j < face.mNumIndices; ++j)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	// Create new mesh with details and return it
	std::unique_ptr<Mesh> newMesh = std::make_unique<Mesh>(device, &vertices, &indices, matToTex[mesh->mMaterialIndex]);

	return newMesh;
}