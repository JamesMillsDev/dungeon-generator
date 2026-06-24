#pragma once

#include <vulkan/vulkan.h>

#include "Graphics/Rendering/GraphicsPipeline.h"
#include "Graphics/Vulkan/DescriptorPool.h"
#include "Graphics/Vulkan/DescriptorWriter.h"
#include "Maths/Alias.h"

class UniformBuffer;
class Texture;

enum class EMaterialPass : uint8
{
	Opaque,
	Transparent,
	Other
};

class Material
{
	friend class Renderer;
	friend class Vulkan;

private:
	GraphicsPipeline* m_pipeline;
	VkDescriptorSetLayout m_layout;
	VkDescriptorSet m_materialSet;
	EMaterialPass m_pass;

	DescriptorWriter m_writer;
	VkDevice m_device;
	GraphicsPipelineConfig m_pipelineConfig;

	Texture** m_textures;
	uint32 m_textureSlots;

public:
	Material(VkDevice device, EMaterialPass pass, GraphicsPipelineConfig pipelineConfig, uint32 textureSlots = 0, Texture** textures = nullptr);
	~Material();

public:
	void WriteMaterial(DescriptorPool& descriptorPool, const UniformBuffer* uniformBuffer);
	void Bind(VkCommandBuffer buffer) const;

private:
	void BuildPipeline();

};