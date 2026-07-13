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

Material::Material(const string& shaderPath) :
	color{ 0xffffffff }, emissiveTint{ 0x00000000 }, roughness{ 0 }, metallic{ 0 },
	specularColor{ Color::WHITE }, specularStrength{ .5f }, baseColorMap{ nullptr },
	normalMap{ nullptr }, ormMap{ nullptr }, emissiveMap{ nullptr },
	m_pipeline{ new VulkanGraphicsPipeline{ GraphicsPipelineConfig{ shaderPath } } },
	m_shouldUpdateDescriptors{ true }
{}

Material::Material(const ShaderConfig& shaderConfig) :
	color{ 0xffffffff }, emissiveTint{ 0x00000000 }, roughness{ 0 }, metallic{ 0 },
	specularColor{ Color::WHITE }, specularStrength{ .5f }, baseColorMap{ nullptr },
	normalMap{ nullptr }, ormMap{ nullptr }, emissiveMap{ nullptr },
	m_pipeline{ new VulkanGraphicsPipeline{ GraphicsPipelineConfig{ shaderConfig } } },
	m_shouldUpdateDescriptors{ true }
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

void Material::Bind(const VkCommandBuffer cmdBuffer, const mat4& transform)
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

	// Bind the pipeline and push the push constants to the command buffer
	m_pipeline->Bind(cmdBuffer, uboBuffer->GetAddress());

	// Update the descriptor sets if needed
	if (m_shouldUpdateDescriptors)
	{
		TList<VkWriteDescriptorSet> writes;

		InsertUniformWrite(writes, sceneLightBuffer, 0);
		InsertUniformWrite(writes, light0Buffer, 1);
		InsertUniformWrite(writes, materialBuffer, 2);

		UpdateDescriptorSets(writes);
		m_shouldUpdateDescriptors = false;
	}
}

void Material::UpdateDescriptorSets(TList<VkWriteDescriptorSet>& writes) const
{
	if (TList<int32> textureBindings; m_pipeline->TryGetTextureBinding(textureBindings))
	{
		TArray textures = { baseColorMap, normalMap, ormMap, emissiveMap };
		for (int64 i = 0; i < textureBindings.Count(); ++i)
		{
			TryInsertTextureDescriptor(writes, textures[i], textureBindings[i]);
		}
	}

	vkUpdateDescriptorSets(Vulkan::Device(), static_cast<uint32>(writes.Count()), writes.Data(), 0, nullptr);
}

void Material::TryInsertTextureDescriptor(TList<VkWriteDescriptorSet>& writes, const Texture* texture, const uint32 binding) const
{
	if (texture != nullptr)
	{
		writes.Add(
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = m_pipeline->GetDescriptorSet(),
				.dstBinding = binding,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo = &texture->GetDescriptors(),
				.pBufferInfo = nullptr,
				.pTexelBufferView = nullptr
			}
		);
	}
}

void Material::InsertUniformWrite(TList<VkWriteDescriptorSet>& writes, const VulkanBuffer* buffer, const uint32 binding, const uint32 arrayElem) const
{
	writes.Add(
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = m_pipeline->GetDescriptorSet(),
			.dstBinding = binding,
			.dstArrayElement = arrayElem,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pImageInfo = nullptr,
			.pBufferInfo = &buffer->GetBufferInfo(),
			.pTexelBufferView = nullptr
		});
}
