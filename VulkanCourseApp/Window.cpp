#include "Window.h"

Window::Window()
	:mName("GLFW Window"), mWidth(1280), mHeight(720), window(nullptr)
{
	defaultInit();
}

Window::~Window()
{
	// Destroy GLFW window and stop GLFW
	glfwDestroyWindow(window);
	glfwTerminate();
}

Window::Window(std::string name, int width, int height)
	:mName(name), mWidth(width), mHeight(height), window(nullptr)
{
	defaultInit();
}

bool Window::createWindow()
{
	window = glfwCreateWindow(mWidth, mHeight, mName.c_str(), nullptr, nullptr);

	return window != nullptr;
}

void Window::defaultInit()
{
	// Initialise GLFW
	glfwInit();

	// Set GLFW to not work with OpenGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	// don't allow window resize to prevent everything being redrawn
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
}
