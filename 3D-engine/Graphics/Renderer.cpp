#include "pch.h"
#include "Renderer.h"

#include "Graphics/Rendering/Mesh.h"
#include "Rendering/Material.h"
#include "Vulkan/Vulkan.h"

Renderer* Renderer::m_instance = nullptr;

Renderer* Renderer::Instance()
{
	return m_instance;
}

bool Renderer::IsValid()
{
	return m_instance != nullptr && Vulkan::IsLoaded();
}

void Renderer::Create(Config* config, GLFWwindow* window)
{
	m_instance = new Renderer{ config, window };
}

void Renderer::Destroy()
{
	delete m_instance;
	m_instance = nullptr;
}

Renderer::Renderer(Config* config, GLFWwindow* window)
{
	m_instance = this;
	InitVulkan(config, window);
}

Renderer::~Renderer()
{
	DestroyVulkan();
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

void Renderer::BeginFrame()
{

}

void Renderer::EndFrame() const
{

}

void Renderer::WaitDeviceIdle() const
{

}

void Renderer::InitVulkan(Config* config, GLFWwindow* window) const
{
	Vulkan::Create(config, window);
}

void Renderer::DestroyVulkan() const
{
	Vulkan::Destroy();
}
