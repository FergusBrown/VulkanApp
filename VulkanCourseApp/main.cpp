#define STB_IMAGE_IMPLEMENTATION
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <iostream>

#include "VulkanRenderer.h"
#include "Pawn.h"
#include "InputHandlerMouse.h"

GLFWwindow* window;
VulkanRenderer vulkanRenderer;

// TODO: function for handling GLFW. Probably want to connect group this together with the input handler
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
	vulkanRenderer.updateCameraView(player.generateView());
	//int frog = vulkanRenderer.createMeshModel("Models/12268_banjofrog_v1_L3.obj");
	int sphere = vulkanRenderer.createMeshModel("Models/sphere.obj");
	mat4 newModel = glm::translate(mat4(1.0f), vec3(0.0f, 2.0f, 0.0f));
	vulkanRenderer.updateModel(sphere, newModel);
	
	int torus = vulkanRenderer.createMeshModel("Models/torus.obj");
	mat4 torusModel = glm::translate(mat4(1.0f), vec3(-2.0f, 2.0f, -2.0f));
	vulkanRenderer.updateModel(torus, torusModel);

	int index = torus + 1;
	for (int i = 0; i < 1; ++i)
	{
		for (int j = 0; j < 10; ++j)
		{
			vulkanRenderer.createMeshModel("Models/torus.obj");
			mat4 tempModel = glm::translate(mat4(1.0f), vec3(-50.0f + i, 4.0f + j, -2.0f));
			vulkanRenderer.updateModel(index, tempModel);
			++index;
		}
	}

	int plane = vulkanRenderer.createMeshModel("Models/blank_plane.obj");
	mat4 rotateMatrix = glm::rotate(mat4(1.0f), glm::radians(90.0f), vec3(0.0f, -1.0f, 0.0f));
	vulkanRenderer.updateModel(plane, rotateMatrix);

	//glm::mat4 testMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(1.0f, 1.0f, 0.0f));
	//vulkanRenderer.updateModel(frog, testMat);

	std::unique_ptr<InputHandler> inputHandler(new InputHandlerMouse(window));
	inputHandler->init();


	// Loop until closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS 
		&& !glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		float now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;
			
		std::vector<CommandPtr> commandList;

		if (inputHandler->handleInput(commandList, deltaTime))
		{
			for (auto& command : commandList)
			{
				command->execute(player);
			}
		}
		vulkanRenderer.updateCameraView(player.generateView());

		torusModel *= glm::rotate(mat4(1.0f), deltaTime, vec3(1.0f, 0.0f, -1.0f));
		vulkanRenderer.updateModel(torus, torusModel);


		vulkanRenderer.draw();
	}

	vulkanRenderer.cleanup();

	// Destroy GLFW window and stop GLFW
	glfwDestroyWindow(window);
	glfwTerminate();


	return 0;
}