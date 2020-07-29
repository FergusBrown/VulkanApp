#pragma once
#include "CameraCommands.h"
#include "ControlConstants.h"

#include <GLFW/glfw3.h>
#include <vector>
#include <memory>

typedef std::unique_ptr<Command> CommandPtr;

class InputHandler
{
public:
	InputHandler(GLFWwindow* window);

	bool handleInput(std::vector<CommandPtr>& commandList);

private:
	GLFWwindow* mWindow;
	double mLastTime;

	// helper
	void handleMouse(std::vector<CommandPtr>& commandList, float deltaTime);
	void handleKeyboard(std::vector<CommandPtr>& commandList, float deltaTime);
};

