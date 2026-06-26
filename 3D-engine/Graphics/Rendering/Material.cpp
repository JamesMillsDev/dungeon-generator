#include "pch.h"
#include "Material.h"

#include "Texture.h"
#include "Graphics/Vulkan/Uniforms.h"
#include "Graphics/Vulkan/Vulkan.h"
#include "Graphics/Vulkan/VulkanBuffer.h"
#include "Graphics/Vulkan/VulkanGraphicsPipeline.h"

Material::Material(const string& shaderPath)
	: color{ 0xffffffff }, emissiveTint{ 0x00000000 }, roughness{ 0 }, metallic{ 0 }, baseColorMap{ nullptr },
	normalMap{ nullptr }, ormMap{ nullptr }, emissiveMap{ nullptr },
	m_pipeline{ new VulkanGraphicsPipeline{ GraphicsPipelineConfig{ shaderPath } } }
{}

Material::~Material()
{
	delete m_pipeline;
	m_pipeline = nullptr;
}

void Material::Render(const VkCommandBuffer cmdBuffer) const
{
	const VulkanBuffer* materialBuffer = Vulkan::Instance()->GetMaterialBuffer();
	const MaterialUniform materialUniform
	{
		.color = color,
		.emissiveTint = emissiveTint,
		.roughness = roughness,
		.metallic = metallic,
		.baseColorMap = baseColorMap != nullptr ? static_cast<int32>(baseColorMap->GetId()) : -1,
		.normalMap = normalMap != nullptr ? static_cast<int32>(normalMap->GetId()) : -1,
		.ormMap = ormMap != nullptr ? static_cast<int32>(ormMap->GetId()) : -1,
		.emissiveMap = emissiveMap != nullptr ? static_cast<int32>(emissiveMap->GetId()) : -1
	};
	materialBuffer->Fill(&materialUniform);

	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->Get());
}
