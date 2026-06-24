#pragma once

#include <vulkan/vulkan.h>

#include "Graphics/Rendering/GraphicsPipeline.h"
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

private:
	GraphicsPipeline* m_pipeline;
	VkDescriptorSetLayout m_layout;
	VkDescriptorSet m_materialSet;
	EMaterialPass m_pass;

	VkDevice m_device;
	GraphicsPipelineConfig m_pipelineConfig;

	vector<Texture*> m_textures;
	uint32 m_textureSlots;

public:
	Material(VkDevice device, EMaterialPass pass, GraphicsPipelineConfig pipelineConfig, uint32 textureSlots);
	~Material();

public:
	void Bind(VkCommandBuffer buffer) const;

	void SetTexture(uint32 slot, Texture* texture);

};