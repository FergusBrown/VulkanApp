#pragma once
#include "Common.h"

class Device;
#include "Buffer.h"

struct Vertex
{
	glm::vec3 position;					// Vertex Position (x, y ,z)
	glm::vec3 normal;					// Vertex Normal (x, y, z)
	glm::vec3 tangent;					// Vertex Tangent (x, y, z)
	glm::vec3 bitangent;				// Vertex Bitangent (x, y, z)
	glm::vec2 uv;						// Texture Coords (u, v)
};

class Mesh
{
public:
	Mesh() = delete;
	Mesh(Device& device, std::vector<Vertex>* vertices, std::vector<uint32_t>* indices,
		uint32_t materialID = 0,
		bool opaque = true);
	~Mesh() = default;
	

	void setModel(glm::mat4 newModel);
	glm::mat4 model() const;

	uint32_t materialID() const;

	uint32_t vertexCount() const;
	Buffer& vertexBuffer();
	
	uint32_t indexCount() const;
	Buffer& indexBuffer();

	bool opaque() const;	// Indicates whether the associated materials are opaque

private:
	glm::mat4 mModel;

	// Texture IDs
	uint32_t mMaterialID;

	// Material info
	bool mOpaque;

	// Vertex and index buffers
	uint32_t mVertexCount;
	std::unique_ptr<Buffer> mVertexBuffer;

	uint32_t mIndexCount;
	std::unique_ptr<Buffer> mIndexBuffer;

	void createVertexBuffer(Device& device, std::vector<Vertex>* vertices);
	void createIndexBuffer(Device& device, std::vector<uint32_t>* indices);
	void copyBuffer(Device& device, Buffer& srcBuffer, Buffer& dstBuffer);
	
};

