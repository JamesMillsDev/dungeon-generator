#pragma once

#include <string>

#include "Common.h"

using std::string;

class Config;
struct GLFWwindow;
class Version;

class Vulkan
{
	friend class Renderer;

private:
	string m_engineTitle;
	Version* m_engineVersion;
	Version* m_vulkanVersion;
	string m_appTitle;
	Version* m_appVersion;

	VkInstance m_instance;

private:
	Vulkan(Config* config, GLFWwindow* window);
	~Vulkan();

private:
	void Create(GLFWwindow* window);
	void Destroy();

	void CreateInstance();

};