#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <vector>
#include <memory>

#include "Utils/Logger.h"
#include "Windows/Window.h"


class App
{
public:
	App();
	~App();
	void Run();

private:
	std::vector<std::unique_ptr<Window>> m_Windows;

	Logger m_Logger;

	GLFWwindow* m_Window;

private:
	void PreFrame();
	void PostFrame();
	void Draw();
	void SetupDockspace();
};