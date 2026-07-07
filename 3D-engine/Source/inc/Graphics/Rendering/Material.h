#pragma once

#include <string>
#include <glm/mat4x4.hpp>
#include <vulkan/vulkan.h>

#include "Object.h"
#include "Maths/Color.h"

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

public:
	explicit Material(const string& shaderPath);
	~Material() override;

public:
	[[nodiscard]] uint64 GetHashCode() const override;

private:
	void Bind(VkCommandBuffer cmdBuffer, const mat4& transform) const;

};