#include "Graphics/Rendering/Material.h"

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

#include "Utility/Collections/HashImpls.h"

Material::Material(const string& shaderPath) :
	color{ 0xffffffff }, emissiveTint{ 0x00000000 }, roughness{ .5f }, metallic{ .5f }, ao{ 1.f },
	m_pipelineConfig{ shaderPath }, m_pipeline{ nullptr }, m_shouldUpdateDescriptors{ true }
{}

Material::Material(const ShaderConfig& shaderConfig) :
	color{ 0xffffffff }, emissiveTint{ 0x00000000 }, roughness{ .5f }, metallic{ .5f }, ao{ 1.f },
	m_pipelineConfig{ shaderConfig }, m_pipeline{ nullptr }, m_shouldUpdateDescriptors{ true }
{}

Material::~Material()
{
	for (Texture*& texture : m_textures)
	{
		delete texture;
	}
	m_textures.Clear();

	delete m_pipeline;
	m_pipeline = nullptr;
}

uint64 Material::GetHashCode() const
{
	return HashAll(color, emissiveTint, roughness, metallic);
}

void Material::AddTexture(Texture* texture)
{
	m_textures.Add(texture);
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

	ImGui::DragFloat("Roughness", &roughness, .01f, 0.f, 1.f, "%.2f");
	ImGui::DragFloat("Metallic", &metallic, .01f, 0.f, 1.f, "%.2f"); 
	ImGui::DragFloat("AO", &ao, .01f, 0.f, FLT_MAX, "%.2f");

	ImGui::End();
}
#endif

void Material::Bind(const VkCommandBuffer cmdBuffer, const mat4& transform)
{
	if (m_pipeline == nullptr)
	{
		for (int64 i = 0; i < m_textures.Count(); ++i)
		{
			VkShaderStageFlagBits stage = VK_SHADER_STAGE_FRAGMENT_BIT; 
			if (m_textures[i]->GetIsNormal())
			{
				// This is a normal map, so make it available in the vertex shader too
				stage = static_cast<VkShaderStageFlagBits>(static_cast<int32>(stage) | VK_SHADER_STAGE_VERTEX_BIT);
			}

			m_pipelineConfig.shaderConfig.descriptors.Add(
				{
					.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.count = 1,
					.stage = static_cast<VkShaderStageFlags>(stage),
					.binding = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
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
		.roughness = roughness,
		.metallic = metallic,
		.ao = ao
	};
	materialBuffer->Fill(&materialUniform);

	// Update the transform buffer with our object's transform
	const MemoryBuffer* pushConstantBuffer = vulkan->GetUniformBuffer(EUniformBufferIds::Transform);

	const mat4 inverted = glm::scale(transform, { 1.f, -1.f, 1.f });
	const TransformUniform transformUniform
	{
		.model = inverted,
		.normal = glm::transpose(glm::inverse(inverted))
	};

	pushConstantBuffer->Fill(&transformUniform);

	// Bind the pipeline and push the push constants to the command buffer
	m_pipeline->Bind(cmdBuffer, pushConstantBuffer->GetAddress());

	// Update the descriptor sets if needed
	if (m_shouldUpdateDescriptors)
	{
		TList<VkWriteDescriptorSet> writes;

		InsertUniformWrite(writes, vulkan->GetUniformBuffer(EUniformBufferIds::ProjectionView), 0);
		InsertUniformWrite(writes, vulkan->GetUniformBuffer(EUniformBufferIds::SceneLighting), 1);

		for (uint8 i = 0; i < MAX_LIGHT_COUNT; ++i)
		{
			if (const MemoryBuffer* buffer = vulkan->GetUniformBuffer(EUniformBufferIds::Lights, i))
			{
				InsertUniformWrite(writes, buffer, 2, i);
			}
		}

		InsertUniformWrite(writes, materialBuffer, 3);

		UpdateDescriptorSets(writes);
		m_shouldUpdateDescriptors = false;
	}
}

void Material::UpdateDescriptorSets(TList<VkWriteDescriptorSet>& writes) const
{
	TList<int32> textureBindings;
	if (m_pipeline->TryGetTextureBinding(textureBindings))
	{
		for (int64 i = 0; i < textureBindings.Count(); ++i)
		{
			InsertTextureWrite(writes, m_textures[i], textureBindings[i]);
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
