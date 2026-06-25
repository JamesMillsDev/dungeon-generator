#pragma once

#include "Maths/Alias.h"
#include "Maths/Color.h"
#include "Maths/Matrix4.h"
#include "Maths/Vector3.h"

struct ProjectionViewUniform
{
	Matrix4 view;
	Matrix4 proj;
};

struct TransformUniform
{
	Matrix4 model;
};

struct LightUniform
{
	uint32 type;
	Vector3 location;
	Vector3 direction;
	Color color;
};