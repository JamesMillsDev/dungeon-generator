#pragma once

#include "Maths/Alias.h"
#include "Maths/Color.h"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

using glm::mat4;
using glm::vec3;

struct ProjectionViewModelUniform
{
	mat4 proj;
	mat4 view;
	mat4 model;
	vec3 cameraLocation;
};

struct MaterialUniform
{
	Color color;
	Color emissiveTint;
	float roughness;
	float metallic;

	int32 baseColorMap;
	int32 normalMap;
	int32 ormMap;
	int32 emissiveMap;
};

struct LightUniform
{
	vec3 location;
	vec3 direction;
	Color color;
	int32 type;
};

struct PushConstantData
{
	VkDeviceAddress uboAddress;
	VkDeviceAddress materialAddress;
	VkDeviceAddress lightingAddress;
};