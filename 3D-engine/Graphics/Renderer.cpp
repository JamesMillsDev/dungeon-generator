#include "pch.h"
#include "Renderer.h"

#include "Graphics/Rendering/Mesh.h"
#include "Rendering/Material.h"
#include "Vulkan/Vulkan.h"

Renderer* Renderer::m_instance = nullptr;

Renderer* Renderer::GetInstance()
{
	return m_instance;
}

Renderer::Renderer(GLFWwindow* window, Config* config)
	: m_vulkan{ new Vulkan{ config } }
{
	m_instance = this;
}

Renderer::~Renderer()
{
	delete m_vulkan;
	m_vulkan = nullptr;
}

void Renderer::Render(const Mesh* mesh, Material* material, const Matrix4& transform)
{

}

Material* Renderer::CreateMaterial(const GraphicsPipelineConfig& config, const EMaterialPass pass, const uint32 textureCount) const
{
	return nullptr;
}

Material* Renderer::CreateMaterial(const string& shaderName, const EMaterialPass pass, const uint32 textureCount) const
{
	return nullptr;
}

bool Renderer::IsValid() const
{
	return m_vulkan != nullptr && m_vulkan->IsLoaded();
}

void Renderer::BeginFrame()
{

}

void Renderer::EndFrame() const
{

}

void Renderer::WaitDeviceIdle() const
{

}
