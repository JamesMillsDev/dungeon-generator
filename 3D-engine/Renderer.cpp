#include "pch.h"
#include "Renderer.h"

#include "Rendering/Mesh.h"
#include "VulkanHelpers/Vulkan.h"

Renderer::Renderer(GLFWwindow* window, Config* config) :
	m_vulkan{ new Vulkan{window, config} }, m_frameCommandBuffer{ VK_NULL_HANDLE }
{}

Renderer::~Renderer()
{
	delete m_vulkan;
	m_vulkan = nullptr;
}

void Renderer::LoadMesh(Mesh* mesh) const
{
	mesh->CreateBuffers(m_vulkan);
}

void Renderer::UnloadMesh(Mesh*& mesh) const
{
	mesh->DestroyBuffer();
	delete mesh;
	mesh = nullptr;
}

void Renderer::RenderMesh(const Mesh* mesh) const
{
	m_vulkan->RecordCommandBuffer(m_frameCommandBuffer, m_vulkan->m_currentImageIndex, [this, mesh]
		{
			mesh->Render(m_frameCommandBuffer);
		});
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

void Renderer::BeginFrame()
{
	m_frameCommandBuffer = m_vulkan->BeginRender();
}

void Renderer::EndFrame() const
{
	if (m_frameCommandBuffer == nullptr)
	{
		return;
	}

	m_vulkan->UpdateUniformBuffer(m_vulkan->m_currentFrame);
	m_vulkan->EndRender();
}

void Renderer::WaitDeviceIdle() const
{
	vkDeviceWaitIdle(m_vulkan->m_device);
}
