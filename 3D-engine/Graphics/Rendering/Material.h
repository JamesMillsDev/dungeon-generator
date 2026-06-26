#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "Maths/Color.h"

class VulkanGraphicsPipeline;
class Texture;

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
	void Render(VkCommandBuffer cmdBuffer) const;

};