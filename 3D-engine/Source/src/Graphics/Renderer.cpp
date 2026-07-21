#include "Graphics/Renderer.h"

#include "Graphics/Rendering/Camera.h"
#include "Graphics/Rendering/Material.h"
#include "Graphics/Rendering/Mesh.h"
#include "Graphics/Vulkan/MemoryBuffer.h"
#include "Graphics/Vulkan/Vulkan.h"

Renderer* Renderer::m_instance = nullptr;
Camera* Renderer::m_currentCamera = nullptr;

Renderer* Renderer::Instance()
{
	return m_instance;
}

bool Renderer::IsValid()
{
	return m_instance != nullptr && Vulkan::IsLoaded();
}

Camera* Renderer::GetCurrentCamera()
{
	return m_currentCamera;
}

void Renderer::SetCurrent(Camera* newCurrent)
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
	: m_frameCmdBuf{ VK_NULL_HANDLE }, m_globalsUniform{  }
{
	m_instance = this;
	InitVulkan(config, window);

	m_vulkan = Vulkan::Instance();
}

Renderer::~Renderer()
{
	DestroyVulkan();
}

void Renderer::Render(const Mesh* mesh, Material* material, const mat4& transform, const uint32 objectIndex) const
{
	TransformUniform transformUniform
	{
		.value = transform
	};

	const MemoryBuffer* transformBuffer = m_vulkan->GetUniformBuffer(EUniformBufferIds::Transforms);
	transformBuffer->Fill(&transformUniform, sizeof(TransformUniform), objectIndex); 

	material->Bind(m_frameCmdBuf, objectIndex);
	mesh->Render(m_frameCmdBuf);
}

void Renderer::BeginFrame()
{
	if (!IsValid())
	{
		return;
	}

	m_frameCmdBuf = m_vulkan->BeginFrame();

	m_currentCamera->GetPvm(m_globalsUniform);

	m_globalsUniform.exposure = 4.5f;
	m_globalsUniform.gamma = 2.2f;
	m_globalsUniform.prefilteredCubeMipLevels = 1.f;
	m_globalsUniform.scaleIBLAmbient = 1.f;

	const MemoryBuffer* globalsBuff = m_vulkan->GetUniformBuffer(EUniformBufferIds::Globals);
	globalsBuff->Fill(&m_globalsUniform);
}

void Renderer::EndFrame()
{
	m_vulkan->EndFrame(m_frameCmdBuf);
	m_frameCmdBuf = VK_NULL_HANDLE;
}