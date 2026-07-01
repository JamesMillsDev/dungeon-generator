#include "pch.h"
#include "Renderer.h"

#include "Application.h"
#include "Window.h"
#include "Graphics/Rendering/Mesh.h"
#include "Rendering/Material.h"
#include "Vulkan/Uniforms.h"
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

ProjectionViewUniform Renderer::ProjectionViewMatrix()
{
	Window* window = Application::GetWindow();

	return 
	{
		.proj = Matrix4::MakePerspective(Maths::Radians(45.f), window->Aspect(), .1f, 32.f),
		.view = Matrix4::MakeLookAt({ 0.f, 0.f, 1.f }, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }),
		.model = Matrix4::Identity(),
		.cameraLocation = { 0.f, 0.f, 1.f },
	};
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

void Renderer::Render(const Mesh* mesh, const Material* material, const Matrix4& transform) const
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