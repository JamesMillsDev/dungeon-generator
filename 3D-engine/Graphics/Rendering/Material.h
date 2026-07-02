#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "Maths/Color.h"

#include <glm/mat4x4.hpp>

class VulkanGraphicsPipeline;
class Texture;

using glm::mat4;
using std::string;
using std::vector;

class Material
{
	friend class Renderer;
		
public:
	Color color;
	Color emissiveTint;
	float roughness;
	float metallic;

	Texture* baseColorMap;
	Texture* normalMap;
	Texture* ormMap;
	Texture* emissiveMap;

private:
	VulkanGraphicsPipeline* m_pipeline;

public:
	explicit Material(const string& shaderPath);
	~Material();

private:
	void Bind(VkCommandBuffer cmdBuffer, const mat4& transform) const;

};