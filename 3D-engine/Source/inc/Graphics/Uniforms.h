#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <vulkan/vulkan_core.h>

using glm::mat4;
using glm::vec3;

struct GlobalsUniform
{
	mat4 proj;
	mat4 view;
	vec3 cameraLocation;

	float exposure;
	float gamma;
	float prefilteredCubeMipLevels;
	float scaleIBLAmbient;
};

struct TransformUniform
{
	mat4 value;
};

struct PushConstants
{
	VkDeviceAddress transform;
	VkDeviceAddress material;
};