#include "pch.h"
#include "Renderer.h"

#include "Graphics/Rendering/Mesh.h"
#include "Graphics/Rendering/Texture.h"
#include "Graphics/Vulkan/Vulkan.h"
#include "Rendering/Material.h"
#include "Vulkan/UniformBuffer.h"

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
	m_vulkan{ new Vulkan{window, config} }, m_frameCommandBuffer{ VK_NULL_HANDLE },
	m_mvpBuffer{ nullptr }, m_descriptorPool{ nullptr }
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
	m_uniformBufferObj.model = transform;
	m_uniformBufferObj.proj[1][1] *= -1.f;

	m_mvpBuffer->Fill(&m_uniformBufferObj);
	
	material->WriteMaterial(*m_descriptorPool, m_mvpBuffer);
	m_vulkan->RecordCommandBuffer(m_frameCommandBuffer, material, [this, mesh]
		{
			mesh->Render(m_frameCommandBuffer);
		});
}

Material* Renderer::CreateMaterial(const GraphicsPipelineConfig& config) const
{
	return new Material{ m_vulkan->m_device, EMaterialPass::Opaque, config, 1 };
}

Material* Renderer::CreateMaterial(const string& shaderName) const
{
	return new Material{ m_vulkan->m_device, EMaterialPass::Opaque, GraphicsPipelineConfig{ shaderName }, 0 };
}

void Renderer::Create()
{
	m_vulkan->Create();

	m_descriptorPool = new DescriptorPool{ m_vulkan->m_device };

	vector<DescriptorPool::PoolSizeRatio> ratios =
	{
		{
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.ratio = 1
		},
		{
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.ratio = 1
		}
	};
	m_descriptorPool->Init(1, ratios);
	m_mvpBuffer = m_vulkan->MakeUniformBuffer(sizeof(UniformBufferObject), 1);
}

void Renderer::Destroy()
{
	if (!IsValid())
	{
		return;
	}

	Vulkan::DestroyBuffer(m_mvpBuffer);

	m_descriptorPool->DestroyPools();
	delete m_descriptorPool;
	m_descriptorPool = nullptr;

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
