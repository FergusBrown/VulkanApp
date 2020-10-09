#pragma once
#include "Common.h"

class Device;
//class Buffer;
#include "Buffer.h"
struct Vertex
{
	glm::vec3 pos;	// Vertex Position (x,y,z)
	glm::vec3 col;	// Vertex Colour
	glm::vec2 tex; // Texture Coords (u, v)
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
	//VkBuffer vertexBuffer;
	//VkDeviceMemory vertexBufferMemory;
	std::unique_ptr<Buffer> mVertexBuffer;
	std::mutex mVertexMutex;
	

	int mIndexCount;
	/*VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;*/
	std::unique_ptr<Buffer> mIndexBuffer;
	std::mutex mIndexMutex;
	//VkPhysicalDevice physicalDevice;
	//VkDevice device;
	

	void createVertexBuffer(Device& device, std::vector<Vertex>* vertices);
	void createIndexBuffer(Device& device, std::vector<uint32_t>* indices);
	void copyBuffer(Device& device, Buffer& srcBuffer, Buffer& dstBuffer);
	
};

