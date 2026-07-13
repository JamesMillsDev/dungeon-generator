#pragma once

#include <string>
#include <glm/mat4x4.hpp>
#include <vulkan/vulkan.h>

#include "Object.h"
#include "Maths/Color.h"

#include "Utility/TList.h"

class VulkanBuffer;
struct ShaderConfig;
class VulkanGraphicsPipeline;
class Texture;

using glm::mat4;
using std::string;

struct MaterialUniform
{
	Color color;
	Color emissiveTint;
	Color specularColor;

	float roughness;
	float metallic;
	float specularStrength;

	int32 baseColorMap;
	int32 normalMap;
	int32 ormMap;
	int32 emissiveMap;
};

class Material : public Object
{
	friend class Renderer;

private:
		
public:
	Color color;
	Color emissiveTint;
	float roughness;
	float metallic;

	Color specularColor;
	float specularStrength;

	Texture* baseColorMap;
	Texture* normalMap;
	Texture* ormMap;
	Texture* emissiveMap;

private:
	VulkanGraphicsPipeline* m_pipeline;
	bool m_shouldUpdateDescriptors;

public:
	explicit Material(const string& shaderPath);
	explicit Material(const ShaderConfig& shaderConfig);
	~Material() override;

public:
	[[nodiscard]] uint64 GetHashCode() const override;

private:
	void Bind(VkCommandBuffer cmdBuffer, const mat4& transform);
	void UpdateDescriptorSets(TList<VkWriteDescriptorSet>& writes) const;

	void TryInsertTextureDescriptor(TList<VkWriteDescriptorSet>& writes, const Texture* texture, uint32 binding) const;
	void InsertUniformWrite(TList<VkWriteDescriptorSet>& writes, const VulkanBuffer* buffer, uint32 binding, uint32 arrayElem = 0) const;

};