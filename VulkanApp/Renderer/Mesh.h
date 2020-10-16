#pragma once
#include "Common.h"

class Device;
//class Buffer;
#include "Buffer.h"

struct Vertex
{
	glm::vec3 position;			// Vertex Position (x, y ,z)
	glm::vec3 normal;			// Vertex Normal (x, y, x)
	glm::vec2 uv;				// Texture Coords (u, v)
};

class Mesh
{
public:
	Mesh() = delete;
	Mesh(Device& device, std::vector<Vertex>* vertices, std::vector<uint32_t>* indices,
		int newTexId);
	~Mesh() = default;
	

	void setModel(glm::mat4 newModel);
	glm::mat4 model() const;

	int texId() const;

	int vertexCount() const;
	Buffer& vertexBuffer();
	
	int indexCount() const;
	Buffer& indexBuffer();

private:
	glm::mat4 mModel;

	int mTexId;

	int mVertexCount;
	std::unique_ptr<Buffer> mVertexBuffer;

	int mIndexCount;
	std::unique_ptr<Buffer> mIndexBuffer;

	void createVertexBuffer(Device& device, std::vector<Vertex>* vertices);
	void createIndexBuffer(Device& device, std::vector<uint32_t>* indices);
	void copyBuffer(Device& device, Buffer& srcBuffer, Buffer& dstBuffer);
	
};

