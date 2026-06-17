#pragma once

class Config;
struct GLFWwindow;
class Vulkan;

class Renderer
{
	friend class Application;

private:
	Vulkan* m_vulkan;

private:
	Renderer(Config* config, GLFWwindow* window);
	~Renderer();

private:

};

