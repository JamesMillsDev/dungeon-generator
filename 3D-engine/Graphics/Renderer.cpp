#include "pch.h"
#include "Renderer.h"

#include "Gameplay/Actors/Components/Rendering/CameraComponent.h"
#include "Graphics/Rendering/Mesh.h"
#include "Rendering/Material.h"
#include "Vulkan/Vulkan.h"

Renderer* Renderer::m_instance = nullptr;
CameraComponent* Renderer::m_currentCamera = nullptr;

Renderer* Renderer::Instance()
{
	return m_instance;
}

bool Renderer::IsValid()
{
	return m_instance != nullptr && Vulkan::IsLoaded();
}

CameraComponent* Renderer::GetCurrentCamera()
{
	return m_currentCamera;
}

void Renderer::SetCurrent(CameraComponent* newCurrent)
{
	if (m_currentCamera != nullptr)
	{
		m_currentCamera->m_isCurrent = false;
	}

	m_currentCamera = newCurrent;
	m_currentCamera->m_isCurrent = true;
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

void Renderer::InitVulkan(Config* config, GLFWwindow* window)
{
	Vulkan::Create(config, window);
}

void Renderer::DestroyVulkan()
{
	Vulkan::Destroy();
}

void Renderer::WaitIdle()
{
	vkDeviceWaitIdle(Vulkan::Device());
}

Renderer::Renderer(Config* config, GLFWwindow* window)
	: m_frameCmdBuf{ VK_NULL_HANDLE }
{
	m_instance = this;
	InitVulkan(config, window);

	m_vulkan = Vulkan::Instance();
}

Renderer::~Renderer()
{
	DestroyVulkan();
}

void Renderer::Render(const Mesh* mesh, const Material* material, const mat4& transform) const
{
	material->Bind(m_frameCmdBuf, transform);

	mesh->Render(m_frameCmdBuf);
}

void Renderer::BeginFrame()
{
	if (!IsValid())
	{
		return;
	}

	m_frameCmdBuf = m_vulkan->BeginFrame();
}

void Renderer::EndFrame()
{
	m_vulkan->EndFrame(m_frameCmdBuf);
	m_frameCmdBuf = VK_NULL_HANDLE;
}