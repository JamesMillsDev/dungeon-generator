#pragma once

#include "Maths/Matrix4.h"

struct ProjectionViewUniform
{
	Matrix4 view;
	Matrix4 proj;
};

struct TransformUniform
{
	Matrix4 model;
};