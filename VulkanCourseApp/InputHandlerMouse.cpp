#include "InputHandlerMouse.h"
#include "ControlConstants.h"

InputHandlerMouse::InputHandlerMouse(GLFWwindow* window)
	: InputHandler(window)
{
	mWindow = InputHandler::mWindow;
}

bool InputHandlerMouse::handleInput(std::vector<CommandPtr>& commandList, float deltaTime)
{
	handleMouse(commandList, deltaTime);
	handleKeyboard(commandList, deltaTime);

	return !commandList.empty();
}

void InputHandlerMouse::init()
{
	// Hide mouse
	glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

void InputHandlerMouse::handleMouse(std::vector<CommandPtr>& commandList, float deltaTime)
{
	int width, height;
	glfwGetWindowSize(mWindow, &width, &height);

	double xCentre = (double) width / 2;
	double yCentre = (double) height / 2;

	// Get mouse position
	double xPos, yPos;
	glfwGetCursorPos(mWindow, &xPos, &yPos);

	// return if no change
	if (xPos == xCentre && yPos == yCentre)
	{
		return;
	}

	float yRotationVal = (xCentre - xPos) * deltaTime * MOUSE_SPEED;
	float xRotationVal = (yCentre - yPos) * deltaTime * MOUSE_SPEED;
	float zRotationVal = 0.0f;

	vec3 eulerAngles = vec3(xRotationVal, yRotationVal, zRotationVal);

	commandList.push_back(CommandPtr(new rotateView(eulerAngles)));

	// Reset mouse position for next frame
	glfwSetCursorPos(mWindow, xCentre, yCentre);
}

void InputHandlerMouse::handleKeyboard(std::vector<CommandPtr>& commandList, float deltaTime)
{
	// Move Up
	if (glfwGetKey(mWindow, GLFW_KEY_SPACE) == GLFW_PRESS) {
   		commandList.push_back(CommandPtr(new MoveUp(MOVE_SPEED * deltaTime)));
	}	
	
	// Move Down
	if (glfwGetKey(mWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		commandList.push_back(CommandPtr(new MoveDown(MOVE_SPEED * deltaTime)));
	}

	// Move Forward
	if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS) {
		commandList.push_back(CommandPtr(new MoveForward(MOVE_SPEED * deltaTime)));
	}

	// Move Back
	if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS) {
		commandList.push_back(CommandPtr(new MoveBack(MOVE_SPEED * deltaTime)));
	}

	// Move Right
	if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS) {
		commandList.push_back(CommandPtr(new MoveRight(MOVE_SPEED * deltaTime)));
	}

	// Move Left
	if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS) {
		commandList.push_back(CommandPtr(new MoveLeft(MOVE_SPEED * deltaTime)));
	}

	// Rotate Left
	if (glfwGetKey(mWindow, GLFW_KEY_LEFT) == GLFW_PRESS) {
		vec3 eulerAngles(0.0f, 0.0f, MOUSE_SPEED * deltaTime);
		commandList.push_back(CommandPtr(new rotateView(eulerAngles)));
	}

	// Rotate Right
	if (glfwGetKey(mWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		vec3 eulerAngles(0.0f, 0.0f, -MOUSE_SPEED * deltaTime);
		commandList.push_back(CommandPtr(new rotateView(eulerAngles)));
	}

}


