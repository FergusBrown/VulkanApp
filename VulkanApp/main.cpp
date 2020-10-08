#include <stdexcept>
#include <vector>
#include <iostream>

#include "VulkanRenderer.h"
#include "Window.h"
#include "Pawn.h"
#include "InputHandlerMouse.h"

int main()
{
	Window displayWindow("Vulkan Renderer", 1280, 720);
	VulkanRenderer vulkanRenderer;

	if (!displayWindow.createWindow())
	{
		return EXIT_FAILURE;
	}

	// Create Vulkan Renderer mInstance
	if (vulkanRenderer.init(displayWindow.window) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	float angle = 0.0f;
	float deltaTime = 0.0f;
	float lastTime = 0.0f;

	Pawn player = Pawn();
	vulkanRenderer.updateCameraView(player.generateView());
	//int frog = vulkanRenderer.loadMeshModelData("Models/12268_banjofrog_v1_L3.obj");
	int sphereData = vulkanRenderer.loadMeshModelData("Models/sphere.obj");
	int sphere = vulkanRenderer.createModel(sphereData);
	mat4 newModel = glm::translate(mat4(1.0f), vec3(0.0f, 2.0f, 0.0f));
	vulkanRenderer.updateModel(sphere, newModel);
	
	int torusData = vulkanRenderer.loadMeshModelData("Models/torus.obj");
	std::vector<int> torusInstances;
	torusInstances.push_back(vulkanRenderer.createModel(torusData));
	mat4 torusModel = glm::translate(mat4(1.0f), vec3(-2.0f, 2.0f, -2.0f));
	vulkanRenderer.updateModel(torusInstances[0], torusModel);

	int index = torusInstances[0] + 1;
	for (int i = 0; i < 100; ++i)
	{
		for (int j = 0; j < 10; ++j)
		{
			torusInstances.push_back(vulkanRenderer.createModel(torusData));
			mat4 tempModel = glm::translate(mat4(1.0f), vec3(-50.0f + i, 4.0f + j, -2.0f));
			vulkanRenderer.updateModel(index, tempModel);
			++index;
		}
	}

	int planeData = vulkanRenderer.loadMeshModelData("Models/blank_plane.obj");
	int plane = vulkanRenderer.createModel(planeData);
	mat4 rotateMatrix = glm::rotate(mat4(1.0f), glm::radians(90.0f), vec3(0.0f, -1.0f, 0.0f));
	vulkanRenderer.updateModel(plane, rotateMatrix);

	//glm::mat4 testMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(1.0f, 1.0f, 0.0f));
	//vulkanRenderer.updateModel(frog, testMat);

	std::unique_ptr<InputHandler> inputHandler(new InputHandlerMouse(displayWindow.window));
	inputHandler->init();


	// Loop until closed
	while (glfwGetKey(displayWindow.window, GLFW_KEY_ESCAPE) != GLFW_PRESS
		&& !glfwWindowShouldClose(displayWindow.window))
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
		vulkanRenderer.updateModel(torusInstances[0], torusModel);


		vulkanRenderer.draw();
	}

	vulkanRenderer.cleanup();

	// Destroy GLFW window and stop GLFW
	//glfwDestroyWindow(displayWindow.window);
	//glfwTerminate();


	return 0;
}