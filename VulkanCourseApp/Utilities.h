#pragma once

#include <fstream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

const int MAX_FRAME_DRAWS = 2;
const int MAX_OBJECTS = 2048;

// TODO : update device creation to take a list of device extensions
// read in additional extensions via file?
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

//struct Vertex
//{
//	glm::vec3 pos;	// Vertex Position (x,y,z)
//	glm::vec3 col;	// Vertex Colour
//	glm::vec2 tex; // Texture Coords (u, v)
//};

// Indicies (locations) of Queue Families (if they exist at all)
//struct QueueFamilyIndices {
//	int graphicsFamily = -1;			// Location of Graphics Queue Family
//	int presentationFamily = -1;
//
//	// Check if queue failies are valid
//	bool isValid()
//	{
//		return graphicsFamily >= 0 && presentationFamily >= 0;
//	}
//};

/* Swapchain*/
//struct SwapChainDetails {
//	VkSurfaceCapabilitiesKHR surfaceCapabilities;		// Surface properties e.g. image size/extent
//	std::vector<VkSurfaceFormatKHR> formats;			// Surface image formats e.g. RGBA and size of each
//	std::vector<VkPresentModeKHR> presentationModes;	// How images should be presented to screen
//};

//struct SwapchainImage {
//	VkImage image;
//	VkImageView imageView;
//};

static std::vector<char> readFile(const std::string& filename)
{
	// Open stream from given file
	// std::ios::binary tells stream to read file as binary
	// std::ios::ate tells stream to start reading from end of file
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	// Check if file stream successfully opened
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open a file!");
	}

	// Get current read position and use to resize file buffer
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> fileBuffer(fileSize);

	// Move read position (seek to) the start of the file
	file.seekg(0);

	// Read the file data into the buffer (stream "fileSize" in total)
	file.read(fileBuffer.data(), fileSize);

	// Close stream
	file.close();

	return fileBuffer;
}


void getWindowExtent(VkExtent2D& windowExtent, GLFWwindow* window)
{
	// Get window size
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	// Create new extent using window size
	windowExtent.width = static_cast<uint32_t>(width);
	windowExtent.height = static_cast<uint32_t>(height);
}
