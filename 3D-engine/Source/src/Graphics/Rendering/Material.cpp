#include "Graphics/Rendering/Material.h"

#include <format>

#include "Gameplay/Actors/Components/Rendering/LightComponent.h"

#include "Graphics/Renderer.h"
#include "Graphics/Uniforms.h"
#include "Graphics/Rendering/Camera.h"
#include "Graphics/Rendering/Lighting.h"
#include "Graphics/Rendering/Texture.h"
#include "Graphics/Vulkan/GraphicsPipeline.h"
#include "Graphics/Vulkan/MemoryBuffer.h"
#include "Graphics/Vulkan/Vulkan.h"

#include "ImGui/imgui.h"

#include "Maths/Maths.h"

#include "Utility/Collections/HashImpls.h"

Material::Material(const string& shaderPath)
	: Material{ ShaderConfig{ .name = shaderPath } }
{
}

Material::Material(const ShaderConfig& shaderConfig)
	: color{ 0xffffffff }, emissiveTint{ 0x00000000 }, ao{ 0.f }, roughness{ .5f }, metallic{ .5f },
	alphaMask{ 1.f }, alphaMaskCutoff{ 0.f }, m_pipelineConfig{ shaderConfig }, m_pipeline{ nullptr },
	m_shouldUpdateDescriptors{ true }
{
	AddTextureMaps();
}

Material::~Material()
{
	for (TMapEntry<string, Texture*>* texture : m_textures)
	{
		delete texture->Value();
	}
	m_textures.Clear();

	delete m_pipeline;
	m_pipeline = nullptr;
}

uint64 Material::GetHashCode() const
{
	return HashAll(color, emissiveTint, ao, roughness, metallic);
}

void Material::SetTexture(const string& id, Texture* texture)
{
	m_textures[id] = texture;
}

#if _DEBUG
void Material::Dbg_ShowGui()
{
	ImGui::Begin("Material");

	float colors[3] = { color.r, color.g, color.b };
	if (ImGui::ColorEdit3("Color", colors))
	{
		color = Color{ colors[0], colors[1], colors[2], color.a };
	}

	colors[0] = emissiveTint.r;
	colors[1] = emissiveTint.g;
	colors[2] = emissiveTint.b;
	if (ImGui::ColorEdit3("Emissive Color", colors))
	{
		emissiveTint = Color{ colors[0], colors[1], colors[2], emissiveTint.a };
	}

	ImGui::DragFloat("AO", &ao, .01f, 0.f, 1.f, "%.2f"); 
	ImGui::DragFloat("Roughness", &roughness, .01f, 0.f, 1.f, "%.2f");
	ImGui::DragFloat("Metallic", &metallic, .01f, 0.f, 1.f, "%.2f"); 

	ImGui::End();
}
#endif

void Material::Bind(const VkCommandBuffer cmdBuffer, const mat4& transform)
{
	if (m_pipeline == nullptr)
	{
		for (const TMapEntry<string, Texture*>* texture : m_textures)
		{
			m_pipelineConfig.shaderConfig.descriptors.Add(
				{
					.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.count = 1,
					.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
					.binding = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
					.name = texture->Key()
				}
			);
		}

		m_pipeline = new GraphicsPipeline{ m_pipelineConfig };
	}

	Vulkan* vulkan = Vulkan::Instance();

	// Update the material uniform with this material's data
	const MemoryBuffer* materialBuffer = vulkan->GetUniformBuffer(EUniformBufferIds::Material);
	const MaterialUniform materialUniform
	{
		.color = color,
		.emissiveTint = emissiveTint,
		.ao = ao,
		.roughness = 1.f - roughness,
		.metallic = metallic,
		.alphaMask = alphaMask,
		.alphaMaskCutoff = alphaMaskCutoff,
		.exposure = 4.5f,
		.gamma = 2.2f,
		.prefilteredCubeMipLevels = 1.f,
		.scaleIBLAmbient = 1.f, 
		.baseColorMap = m_textures[BASE_COLOR_MAP_NAME] != nullptr ? 1 : 0,
		.normalMap = m_textures[NORMAL_MAP_NAME] != nullptr ? 1 : 0,
		.ormMap = m_textures[ORM_MAP_NAME] != nullptr ? 1 : 0,
		.emissiveMap = m_textures[EMISSIVE_MAP_NAME] != nullptr ? 1 : 0,
		.heightMap = m_textures[HEIGHT_MAP_NAME] != nullptr ? 1 : 0,
	};
	materialBuffer->Fill(&materialUniform);

	// Bind the pipeline and push the push constants to the command buffer
	m_pipeline->Bind(cmdBuffer, transform);

	// Update the descriptor sets if needed
	if (m_shouldUpdateDescriptors)
	{
		TList<VkWriteDescriptorSet> writes;

		InsertUniformWrite(
			writes, vulkan->GetUniformBuffer(EUniformBufferIds::ProjectionView), 
			static_cast<uint32>(EUniformBufferIds::ProjectionView)
		);
		InsertUniformWrite(
			writes, vulkan->GetUniformBuffer(EUniformBufferIds::SceneLighting),
			static_cast<uint32>(EUniformBufferIds::SceneLighting)
		);

		for (uint8 i = 0; i < MAX_LIGHT_COUNT; ++i)
		{
			if (const MemoryBuffer* buffer = vulkan->GetUniformBuffer(EUniformBufferIds::Lights, i))
			{
				InsertUniformWrite(writes, buffer, static_cast<uint32>(EUniformBufferIds::Lights), i);
			}
		}

		InsertUniformWrite(
			writes, materialBuffer, static_cast<uint32>(EUniformBufferIds::Material)
		);

		UpdateDescriptorSets(writes);
		m_shouldUpdateDescriptors = false;
	}
}

void Material::UpdateDescriptorSets(TList<VkWriteDescriptorSet>& writes) const
{
	TMap<string, int32> textureBindings;
	if (m_pipeline->TryGetTextureBinding(textureBindings))
	{
		for (TMapEntry<string, Texture*>* texture : m_textures)
		{
			if (texture->Value() != nullptr)
			{
				InsertTextureWrite(writes, texture->Value(), textureBindings[texture->Key()]);
			}
		}
	}

	vkUpdateDescriptorSets(Vulkan::Device(), static_cast<uint32>(writes.Count()), writes.Data(), 0, nullptr);
}

void Material::InsertTextureWrite(TList<VkWriteDescriptorSet>& writes, const Texture* texture, const uint32 binding) const
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

void Material::InsertUniformWrite(TList<VkWriteDescriptorSet>& writes, const MemoryBuffer* buffer, const uint32 binding, const uint32 arrayElem) const
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

void Material::AddTextureMaps()
{
	m_textures.Add(BASE_COLOR_MAP_NAME, nullptr);
	m_textures.Add(NORMAL_MAP_NAME, nullptr);
	m_textures.Add(ORM_MAP_NAME, nullptr);
	m_textures.Add(EMISSIVE_MAP_NAME, nullptr);
	m_textures.Add(HEIGHT_MAP_NAME, nullptr);
}
