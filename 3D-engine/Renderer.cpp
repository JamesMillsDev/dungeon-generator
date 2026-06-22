#include "pch.h"
#include "Renderer.h"

#include "Rendering/Mesh.h"
#include "Rendering/Texture.h"
#include "VulkanHelpers/Vulkan.h"

Renderer* Renderer::m_instance = nullptr;

Renderer* Renderer::GetInstance()
{
	return m_instance;
}

void Renderer::Load(Mesh* mesh)
{
	mesh->CreateBuffers();
}

void Renderer::Unload(Mesh*& mesh)
{
	mesh->DestroyBuffers();
	delete mesh;
	mesh = nullptr;
}

void Renderer::Load(Texture* texture)
{
	texture->CreateBuffer();
}

void Renderer::Unload(Texture*& texture)
{
	texture->DestroyBuffer();
	delete texture;
	texture = nullptr;
}

Renderer::Renderer(GLFWwindow* window, Config* config) :
	m_vulkan{ new Vulkan{window, config} }, m_frameCommandBuffer{ VK_NULL_HANDLE }
{
	m_instance = this;
}

Renderer::~Renderer()
{
	delete m_vulkan;
	m_vulkan = nullptr;
}

void Renderer::Render(const Mesh* mesh) const
{
	m_vulkan->RecordCommandBuffer(m_frameCommandBuffer, m_vulkan->m_currentImageIndex, [this, mesh]
	{
		mesh->Render(m_frameCommandBuffer);
	}, 0);
}

void Renderer::Create() const
{
	m_vulkan->Create(vector<GraphicsPipelineConfig>
		{
			vector<ShaderConfig>
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
	m_vulkan->UpdateUniformBuffer(m_vulkan->m_currentFrame);
}

void Renderer::EndFrame() const
{
	if (m_frameCommandBuffer == nullptr)
	{
		return;
	}

	m_vulkan->EndRender();
}

void Renderer::WaitDeviceIdle() const
{
	vkDeviceWaitIdle(m_vulkan->m_device);
}
