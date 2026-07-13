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

void Material::TryInsertTextureDescriptor(TList<VkDescriptorImageInfo>& descriptors, Texture* texture)
{
	if (texture != nullptr)
	{
		descriptors.Add(texture->GetDescriptors());
	}
}

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

		VkDescriptorBufferInfo materialDescriptor
		{
			.buffer = materialBuffer->Get(),
			.offset = 0,
			.range = materialBuffer->Size()
		};
		writes.Add(GetUniformWrite(&materialDescriptor, 2));

		VkDescriptorBufferInfo lightDescriptor
		{
			.buffer = light0Buffer->Get(),
			.offset = 0,
			.range = light0Buffer->Size()
		};
		writes.Add(GetUniformWrite(&lightDescriptor, 1));

		VkDescriptorBufferInfo sceneDescriptor
		{
			.buffer = sceneLightBuffer->Get(),
			.offset = 0,
			.range = sceneLightBuffer->Size()
		};
		writes.Add(GetUniformWrite(&sceneDescriptor, 0));

		UpdateDescriptorSets(writes);
		m_shouldUpdateDescriptors = false;
	}
}

void Material::UpdateDescriptorSets(TList<VkWriteDescriptorSet>& writes) const
{
	TList<VkDescriptorImageInfo> textureDescriptors;
	if (int32 textureBinding; m_pipeline->TryGetTextureBinding(textureBinding))
	{
		TryInsertTextureDescriptor(textureDescriptors, baseColorMap);
		TryInsertTextureDescriptor(textureDescriptors, normalMap);
		TryInsertTextureDescriptor(textureDescriptors, ormMap);
		TryInsertTextureDescriptor(textureDescriptors, emissiveMap);

		writes.Add(
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = m_pipeline->GetDescriptorSet(),
				.dstBinding = static_cast<uint32>(textureBinding),
				.dstArrayElement = 0,
				.descriptorCount = static_cast<uint32>(textureDescriptors.size()),
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo = textureDescriptors.Data(),
				.pBufferInfo = nullptr,
				.pTexelBufferView = nullptr
			}
		);
	}

	vkUpdateDescriptorSets(Vulkan::Device(), static_cast<uint32>(writes.Count()), writes.Data(), 0, nullptr);
}

VkWriteDescriptorSet Material::GetUniformWrite(VkDescriptorBufferInfo* buffer, const uint32 binding, uint32 arrayElem) const
{
	return
	{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = nullptr,
		.dstSet = m_pipeline->GetDescriptorSet(),
		.dstBinding = binding,
		.dstArrayElement = arrayElem,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.pImageInfo = nullptr, 
		.pBufferInfo = buffer,
		.pTexelBufferView = nullptr
	};
}
