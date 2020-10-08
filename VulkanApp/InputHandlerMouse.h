#pragma once
#include "InputHandler.h"


class InputHandlerMouse : public InputHandler
{
public:
	InputHandlerMouse(GLFWwindow* window);

	virtual bool handleInput(std::vector<CommandPtr>& commandList, float deltaTime);
	void init();

private:
	GLFWwindow* mWindow;

	// helper
	void handleMouse(std::vector<CommandPtr>& commandList, float deltaTime);
	void handleKeyboard(std::vector<CommandPtr>& commandList, float deltaTime);
};

