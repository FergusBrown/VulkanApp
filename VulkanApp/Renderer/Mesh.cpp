#include "Mesh.h"

#include "Device.h"
#include "CommandBuffer.h"

Mesh::Mesh(Device& device,
	std::vector<Vertex>* vertices, std::vector<uint32_t> * indices,
	uint32_t materialID,
	bool opaque) :
	mVertexCount(vertices->size()), mIndexCount(indices->size()),
	mMaterialID(materialID),
	mModel(glm::mat4(1.0f)),
	mOpaque(opaque)
{
	createVertexBuffer(device, vertices);
	createIndexBuffer(device, indices);
}

void Mesh::setModel(glm::mat4 newModel)
{
	mModel = newModel;
}

glm::mat4 Mesh::model() const
{
	return mModel;
}

uint32_t Mesh::materialID() const
{
	return mMaterialID;
}


uint32_t Mesh::vertexCount() const
{
	return mVertexCount;
}

Buffer& Mesh::vertexBuffer()
{
	//std::lock_guard<std::mutex> lock(mVertexMutex);
	return *mVertexBuffer;
}

uint32_t Mesh::indexCount() const
{
	return mIndexCount;
}

Buffer& Mesh::indexBuffer()
{
	//std::lock_guard<std::mutex> lock(mIndexMutex);
	return *mIndexBuffer;
}

bool Mesh::opaque() const
{
	return mOpaque;
}

// TODO : could abstract this and the vertex buffer creation to a template function
void Mesh::createVertexBuffer(Device& device, std::vector<Vertex>* vertices)
{
	VkDeviceSize bufferSize = sizeof(Vertex) * vertices->size();

	// Create staging buffer to hold loaded data, ready to copy to device
	Buffer stagingBuffer(device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	
	// Copy vertex data to staging buffer
	void* data;											// 1. create pointer to a point in normal memory
	data = stagingBuffer.map();							// 2. "Map" the vertex buffer memory to that point												
	memcpy(data, vertices->data(), (size_t)bufferSize);	// 3. Copy Memory from vertices data to the point
	stagingBuffer.unmap();								// 4. Unmap the vertex buffer memory

	// Create buffer with TRANSFER_DST_BIT to mark as a recipient of transfer data
	// Buffer memory is to be DEVICE_LOCAL_BIT meabning memory is on the GPU and only accessible by it and not CPU (host)
	mVertexBuffer = std::make_unique<Buffer>(device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// Copy staging buffer to vertex buffer on GPU
	copyBuffer(device, stagingBuffer, *mVertexBuffer);
}

// TODO : could abstract this and the vertex buffer creation to a template function
void Mesh::createIndexBuffer(Device& device, std::vector<uint32_t>* indices)
{
	VkDeviceSize bufferSize = sizeof(uint32_t) * indices->size();

	// Create staging buffer to hold loaded data, ready to copy to device
	Buffer stagingBuffer(device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Copy vertex data to staging buffer
	void* data;											// 1. create pointer to a point in normal memory
	data = stagingBuffer.map();							// 2. "Map" the index buffer memory to that point												
	memcpy(data, indices->data(), (size_t)bufferSize);	// 3. Copy Memory from vertices data to the point
	stagingBuffer.unmap();								// 4. Unmap the index buffer memory

	// Create buffer with TRANSFER_DST_BIT to mark as a recipient of transfer data
	// Buffer memory is to be DEVICE_LOCAL_BIT meabning memory is on the GPU and only accessible by it and not CPU (host)
	mIndexBuffer = std::make_unique<Buffer>(device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// Copy staging buffer to vertex buffer on GPU
	copyBuffer(device, stagingBuffer, *mIndexBuffer);
}

void Mesh::copyBuffer(Device& device, Buffer& srcBuffer, Buffer& dstBuffer)
{
	std::unique_ptr<CommandBuffer> commandBuffer = device.createAndBeginTemporaryCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	commandBuffer->copyBuffer(srcBuffer, dstBuffer);

	device.endAndSubmitTemporaryCommandBuffer(*commandBuffer);
}
