#include "pch.h"
#include "Renderer.h"

#include "VulkanHelpers/Vulkan.h"

Renderer::Renderer(Config* config)
	: m_vulkan{ new Vulkan{ config } }
{
}

Renderer::~Renderer()
{
	delete m_vulkan;
	m_vulkan = nullptr;
}

void Renderer::Create(GLFWwindow* window) const
{
	m_vulkan->Create(window);
}

void Renderer::Destroy() const
{
	if (!IsValid())
	{
		return;
	}

	m_vulkan->Destroy();
}

bool Renderer::IsValid() const
{
	return m_vulkan != nullptr && m_vulkan->m_loaded;
}
