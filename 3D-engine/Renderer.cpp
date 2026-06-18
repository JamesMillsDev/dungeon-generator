#include "pch.h"
#include "Renderer.h"

#include "VulkanHelpers/Vulkan.h"

Renderer::Renderer(GLFWwindow* window, Config* config)
	: m_vulkan{ new Vulkan{ window, config } }
{
}

Renderer::~Renderer()
{
	delete m_vulkan;
	m_vulkan = nullptr;
}

void Renderer::Create() const
{
	m_vulkan->Create( 
		{
			{
				{.stage = VK_SHADER_STAGE_VERTEX_BIT,   .shader = "Triangle.vert" },
				{.stage = VK_SHADER_STAGE_FRAGMENT_BIT, .shader = "Triangle.frag" }
			}
		}
	);
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

void Renderer::RenderFrame() const
{
	m_vulkan->RenderFrame();
}

void Renderer::WaitDeviceIdle() const
{
	vkDeviceWaitIdle(m_vulkan->m_device);
}
