#include "pch.h"
#include "Renderer.h"

#include "Graphics/Rendering/Mesh.h"
#include "Graphics/Rendering/Texture.h"
#include "Graphics/Vulkan/Vulkan.h"

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

void Renderer::Render(const Mesh* mesh, const GraphicsPipeline* pipeline, const Matrix4& transform)
{
	m_uniformBufferObj.model = transform;
	m_uniformBufferObj.proj[1][1] *= -1.f;

	m_vulkan->UpdateUniformBuffer(m_vulkan->m_currentFrame, &m_uniformBufferObj);
	m_vulkan->RecordCommandBuffer(m_frameCommandBuffer, pipeline, [this, mesh]
		{
			mesh->Render(m_frameCommandBuffer);
		});
}

GraphicsPipeline* Renderer::CreatePipeline(const GraphicsPipelineConfig& config) const
{
	return m_vulkan->CreatePipeline(config);
}

GraphicsPipeline* Renderer::CreatePipeline(const string& shaderName) const
{
	return m_vulkan->CreatePipeline(GraphicsPipelineConfig{ shaderName });
}

void Renderer::Create() const
{
	m_vulkan->Create();
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

	m_uniformBufferObj =
	{
		.model = Matrix4::Identity(),
		.view = Matrix4::MakeLookAt({ 2.f, 2.f, 2.f }, { 0.f, 0.f, 0.f }, { 0.f, 0.f, 1.f }),
		.proj = Matrix4::MakePerspective(
			Maths::Radians(45.f),
			static_cast<float>(m_vulkan->m_swapChainExtent.width) / static_cast<float>(m_vulkan->m_swapChainExtent.height),
			.1f, 10.f
		)
	};
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
