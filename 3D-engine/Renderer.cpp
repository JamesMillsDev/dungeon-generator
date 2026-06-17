#include "pch.h"
#include "Renderer.h"

#include "VulkanHelpers/Vulkan.h"

Renderer::Renderer(Config* config, GLFWwindow* window)
	: m_vulkan{ new Vulkan{ config, window } }
{
}

Renderer::~Renderer()
{
	m_vulkan->Destroy();
	delete m_vulkan;
	m_vulkan = nullptr;
}
