#define STB_IMAGE_IMPLEMENTATION
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <iostream>

#include "VulkanRenderer.h"
#include "Pawn.h"
#include "InputHandler.h"

GLFWwindow* window;
VulkanRenderer vulkanRenderer;

// function for handling GLFW. Probably want a class to do this if also handling input
void initWindow(std::string wName = "Test Window", const int width = 800, const int height = 600)
{
	// Initialise GLFW
	glfwInit();

	// Set GLFW to not work with OpenGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	// don't allow window resize to prevent everything being redrawn
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	// note c_str converts a cpp string to a c string
	window = glfwCreateWindow(width, height, wName.c_str(), nullptr, nullptr);
}

int main()
{
	// create window
	initWindow("Test Window", 1280, 720);

	// Create Vulkan Renderer instance
	if (vulkanRenderer.init(window) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	float angle = 0.0f;
	float deltaTime = 0.0f;
	float lastTime = 0.0f;

	Pawn player = Pawn();
	//vulkanRenderer.createCamera(90.0f);
	vulkanRenderer.updateCameraView(player.generateView());
	int frog = vulkanRenderer.createMeshModel("Models/12268_banjofrog_v1_L3.obj");

	InputHandler inputHandler = InputHandler(window);
	
	// Loop until closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS 
		&& !glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		float now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;

		angle += 100.0f * deltaTime;
		if (angle > 360.0f)
		{
			angle -= 360.0f;
		}

		
		//glm::mat4 testMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(1.0f, 1.0f, 0.0f));
		//testMat = glm::rotate(testMat, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//vulkanRenderer.updateModel(frog, testMat);

		std::vector<CommandPtr> commandList;

		if (inputHandler.handleInput(commandList))
		{
			for (auto& command : commandList)
			{
				command->execute(player);
			}
		}
		vulkanRenderer.updateCameraView(player.generateView());
		vulkanRenderer.draw();
	}

	vulkanRenderer.cleanup();

	// Destroy GLFW window and stop GLFW
	glfwDestroyWindow(window);
	glfwTerminate();


	return 0;
}