#pragma once
#include <GLFW/glfw3.h>
#include <vector>

#include "Commands.h"

typedef std::unique_ptr<Command> CommandPtr;

class InputHandler
{
public:
	InputHandler(GLFWwindow* window) :
		mWindow(window)
	{
	};

	virtual void init() = 0;
	virtual bool handleInput(std::vector<CommandPtr>& commandList, float deltaTime) = 0;

protected:
	GLFWwindow* mWindow;
};