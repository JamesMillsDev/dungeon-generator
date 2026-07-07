#include "Graphics/Rendering/Material.h"

#include "Gameplay/Actors/Components/Rendering/LightComponent.h"

#include "Graphics/Renderer.h"
#include "Graphics/Rendering/Camera.h"
#include "Graphics/Rendering/SceneLightingData.h"
#include "Graphics/Rendering/Texture.h"
#include "Graphics/Vulkan/Uniforms.h"
#include "Graphics/Vulkan/Vulkan.h"
#include "Graphics/Vulkan/VulkanBuffer.h"
#include "Graphics/Vulkan/VulkanGraphicsPipeline.h"

#include "Utility/HashImpls.h"

Material::Material(const string& shaderPath)
	: color{ 0xffffffff }, emissiveTint{ 0x00000000 }, roughness{ 0 }, metallic{ 0 },
	specularColor{ Color::WHITE }, specularStrength{ .5f }, baseColorMap{ nullptr },
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

uint64 Material::GetHashCode() const
{
	return HashAll(
		color, emissiveTint, roughness, metallic,
		specularColor, specularStrength
	);
}

void Material::Bind(const VkCommandBuffer cmdBuffer, const mat4& transform) const
{
	// Update the material uniform with this material's data
	const VulkanBuffer* materialBuffer = Vulkan::Instance()->GetUniformBuffer(EUniformBufferIds::Material);
	const MaterialUniform materialUniform
	{
		.color = color,
		.emissiveTint = emissiveTint,
		.specularColor = specularColor,
		.roughness = roughness,
		.metallic = metallic,
		.specularStrength = specularStrength,
		.baseColorMap = baseColorMap != nullptr ? static_cast<int32>(baseColorMap->GetId()) : -1,
		.normalMap = normalMap != nullptr ? static_cast<int32>(normalMap->GetId()) : -1,
		.ormMap = ormMap != nullptr ? static_cast<int32>(ormMap->GetId()) : -1,
		.emissiveMap = emissiveMap != nullptr ? static_cast<int32>(emissiveMap->GetId()) : -1
	};
	materialBuffer->Fill(&materialUniform);

	// Update the transform buffer with our object's transform
	const VulkanBuffer* uboBuffer = Vulkan::Instance()->GetUniformBuffer(EUniformBufferIds::ProjectionView);
	ProjectionViewModelUniform pvm;
	Renderer::GetCurrentCamera()->GetPvm(pvm);

	pvm.model = transform;
	uboBuffer->Fill(&pvm);

	// TODO: Use more dynamic lighting. This is a test
	const VulkanBuffer* light0Buffer = Vulkan::Instance()->GetUniformBuffer(EUniformBufferIds::Lights, 0);
	LightUniform light0
	{
		.location = { 0.f, 0.f, 0.f },
		.direction = { .32f, -.77f, -.56f }, // this is the unity default light direction
		.color = Color::WHITE,
		.type = 0,
	};
	light0Buffer->Fill(&light0);

	const VulkanBuffer* sceneLightBuffer = Vulkan::Instance()->GetUniformBuffer(EUniformBufferIds::SceneLighting);
	SceneLightingData sceneLighting
	{
		.ambientColor = Color::WHITE,
		.ambientStrength = .1f
	};
	sceneLightBuffer->Fill(&sceneLighting);

	// Send off the push constant pointers
	const VulkanBuffer* pushConstantBuffer = Vulkan::Instance()->GetUniformBuffer(EUniformBufferIds::PushConstant);
	PushConstantData pushConstantData
	{
		.uboAddress = uboBuffer->GetAddress(),
		.materialAddress = materialBuffer->GetAddress(),
		.sceneLightingAddress = sceneLightBuffer->GetAddress(),
		.lightsAddress =
		{
			light0Buffer->GetAddress(), 0, 0, 0, 0, 0, 0, 0
		}
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
