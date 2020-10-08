#pragma once
#include <GLFW/glfw3.h>

#include <string>

class Window
{
public:
	Window();
	~Window();
	Window(std::string name, int width, int height);

	bool createWindow();



	GLFWwindow* window;
private:
	std::string mName;
	int mWidth;
	int mHeight;

	void defaultInit();
};

