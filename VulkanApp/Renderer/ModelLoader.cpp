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

		//vertices[i].tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };

		// Set tex coords (if they exist)
		if (mesh->mTextureCoords[0])
		{
			vertices[i].uv = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
		}
		else
		{
			vertices[i].uv = { 0.0f, 0.0f };
		}
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

	// Get tangent and bitangent
	calculateTangentBasis(vertices, indices);

	// Create new mesh with details and return it
	std::unique_ptr<Mesh> newMesh = std::make_unique<Mesh>(device, &vertices, &indices, matToTex[mesh->mMaterialIndex]);

	return newMesh;
}

void calculateTangentBasis(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	for (size_t i = 0; i < indices.size(); i += 3)
	{
		uint32_t index0 = indices[i];
		uint32_t index1 = indices[i + 1];
		uint32_t index2 = indices[i + 2];

		glm::vec3& pos0 = vertices[index0].position;
		glm::vec3& pos1 = vertices[index1].position;
		glm::vec3& pos2 = vertices[index2].position;

		glm::vec2& uv0 = vertices[index0].uv;
		glm::vec2& uv1 = vertices[index1].uv;
		glm::vec2& uv2 = vertices[index2].uv;

		// Edges of the triangle
		glm::vec3 deltaPos1 = pos1 - pos0;
		glm::vec3 deltaPos2 = pos2 - pos0;

		// UV delta (orient tangent in same direction as uv coords)
		glm::vec2 deltaUV1 = uv1 - uv0;
		glm::vec2 deltaUV2 = uv2 - uv0;

		// Calculate tangent and bitangent
		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
		glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r; 

		vertices[index0].tangent = tangent;
		vertices[index1].tangent = tangent;
		vertices[index2].tangent = tangent;

		vertices[index0].bitangent = bitangent;
		vertices[index1].bitangent = bitangent;
		vertices[index2].bitangent = bitangent;
	}
	
}
