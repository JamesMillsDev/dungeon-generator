#include "pch.h"
#include "Material.h"

#include "Texture.h"
#include "Uniforms.h"
#include "Graphics/Vulkan/DescriptorLayoutBuilder.h"
#include "Graphics/Vulkan/UniformBuffer.h"
#include "Graphics/Vulkan/Vulkan.h"

Material::Material(const VkDevice device, const EMaterialPass pass, GraphicsPipelineConfig pipelineConfig,
	const uint32 textureSlots, Texture** textures)
	: m_pipeline{ nullptr }, m_layout{ VK_NULL_HANDLE }, m_materialSet{ VK_NULL_HANDLE },
	m_pass{ pass }, m_writer{ device }, m_device{ device },
	m_pipelineConfig{ std::move(pipelineConfig) }, m_textures{ textures },
	m_textureSlots{ textureSlots }
{
	BuildPipeline();
}

Material::~Material()
{
	vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);

	delete m_pipeline;
	m_pipeline = nullptr;
}

void Material::WriteMaterial(DescriptorPool& descriptorPool, const UniformBuffer* uniformBuffer)
{
	m_materialSet = descriptorPool.Allocate(m_layout);

	m_writer.WriteBuffer(0, uniformBuffer->Get(), sizeof(UniformBufferObject), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

	for (uint32 i = 0; i < m_textureSlots; ++i)
	{
		m_textures[i]->Write(i + 1, m_writer);
	}

	m_writer.UpdateSet(m_materialSet);
}

void Material::Bind(const VkCommandBuffer buffer) const
{
	vkCmdBindDescriptorSets(
		buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->GetLayout(),
		0, m_textureSlots + 1, &m_materialSet, 0, nullptr
	);

	m_pipeline->Bind(buffer);
}

void Material::BuildPipeline()
{
	Vulkan* vulkan = Vulkan::Instance();

	DescriptorLayoutBuilder builder;
	builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	for (uint32 i = 0; i < m_textureSlots; i++)
	{
		builder.AddBinding(i + 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	}

	m_layout = builder.Build(m_device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
	m_pipeline = vulkan->CreatePipeline(m_pipelineConfig, m_layout);
}
