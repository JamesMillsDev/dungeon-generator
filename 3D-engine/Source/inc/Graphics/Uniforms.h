#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

using glm::mat4;
using glm::vec3;

struct GlobalsUniform
{
	mat4 proj;
	mat4 view;
	mat4 transform;
	vec3 cameraLocation;

	float exposure;
	float gamma;
	float prefilteredCubeMipLevels;
	float scaleIBLAmbient;
};