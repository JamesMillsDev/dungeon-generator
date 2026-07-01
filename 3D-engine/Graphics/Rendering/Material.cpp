#include "pch.h"
#include "Material.h"

#include "Texture.h"
#include "Graphics/Renderer.h"
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
	delete baseColorMap;
	delete normalMap;
	delete ormMap;
	delete emissiveMap;

	delete m_pipeline;
	m_pipeline = nullptr;
}

void Material::Bind(const VkCommandBuffer cmdBuffer, const Matrix4& transform) const
{
	// Update the material uniform with this material's data
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

	// Update the transform buffer with our object's transform
	const VulkanBuffer* uboBuffer = Vulkan::Instance()->GetUboBuffer();
	ProjectionViewUniform ubo = Renderer::ProjectionViewMatrix();
	ubo.model = transform;
	uboBuffer->Fill(&ubo);

	// TODO: Use more dynamic lighting. This is a test
	const VulkanBuffer* lightBuffer = Vulkan::Instance()->GetLightBuffer();
	LightUniform lighting
	{
		.location = { 0.f, 0.f, 0.f },
		.direction = { -.32f, -.77f, .56f }, // this is the unity default light direction
		.color = Color::WHITE,
		.type = 0,
	};
	lightBuffer->Fill(&lighting);

	// Send off the push constant pointers
	const VulkanBuffer* pushConstantBuffer = Vulkan::Instance()->GetPushConstantBuffer();
	PushConstantData pushConstantData
	{
		.uboAddress = uboBuffer->GetAddress(),
		.materialAddress = materialBuffer->GetAddress(),
		.lightingAddress = lightBuffer->GetAddress()
	};
	pushConstantBuffer->Fill(&pushConstantData);

	// Bind the pipeline and push the push constants to the command buffer
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->Get());
	Vulkan::Instance()->BindTextureDescriptorSets(cmdBuffer, m_pipeline->GetLayout());

	vkCmdPushConstants(
		cmdBuffer, m_pipeline->GetLayout(), VK_SHADER_STAGE_ALL_GRAPHICS, 0,
		sizeof(PushConstantData), &pushConstantBuffer->GetAddress()
	);
}
