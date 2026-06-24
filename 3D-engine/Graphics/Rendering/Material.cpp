#include "pch.h"
#include "Material.h"

#include "Texture.h"

Material::Material(const VkDevice device, const EMaterialPass pass, GraphicsPipelineConfig pipelineConfig,
	const uint32 textureSlots)
	: m_pipeline{ nullptr }, m_layout{ VK_NULL_HANDLE }, m_materialSet{ VK_NULL_HANDLE },
	m_pass{ pass }, m_device{ device }, m_pipelineConfig{ std::move(pipelineConfig) },
	m_textureSlots{ textureSlots }
{
	m_textures.resize(m_textureSlots);
}

Material::~Material()
{
	vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);

	delete m_pipeline;
	m_pipeline = nullptr;
}

void Material::Bind(const VkCommandBuffer buffer) const
{
	vkCmdBindDescriptorSets(
		buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->GetLayout(),
		0, m_textureSlots + 1, &m_materialSet, 0, nullptr
	);

	m_pipeline->Bind(buffer);
}

void Material::SetTexture(const uint32 slot, Texture* texture)
{
	m_textures[slot] = texture;
}
