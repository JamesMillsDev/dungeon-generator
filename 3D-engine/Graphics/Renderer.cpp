#include "pch.h"
#include "Renderer.h"

#include "Graphics/Rendering/Mesh.h"
#include "Rendering/Material.h"

Renderer* Renderer::m_instance = nullptr;

Renderer* Renderer::GetInstance()
{
	return m_instance;
}

Renderer::Renderer(GLFWwindow* window, Config* config) :
	m_frameCommandBuffer{ VK_NULL_HANDLE }, m_mvpBuffer{ nullptr }
{
	m_instance = this;
}

Renderer::~Renderer()
{
	
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

void Renderer::Create()
{
	
}

void Renderer::Destroy()
{
	if (!IsValid())
	{
		return;
	}

	
}

bool Renderer::IsValid() const
{
	return false;
}

void Renderer::BeginFrame()
{
	
}

void Renderer::EndFrame() const
{
	if (m_frameCommandBuffer == nullptr)
	{
		return;
	}


}

void Renderer::WaitDeviceIdle() const
{

}
