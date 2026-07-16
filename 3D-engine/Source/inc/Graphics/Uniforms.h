#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

using glm::mat4;
using glm::vec3;

struct ProjectionViewUniform
{
	mat4 proj;
	mat4 view;
	vec3 cameraLocation;
};

struct TransformUniform
{
	mat4 model;
	mat4 normal;
};