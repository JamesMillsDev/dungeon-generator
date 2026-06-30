#pragma once

#include "Maths/Alias.h"
#include "Maths/Color.h"
#include "Maths/Matrix4.h"
#include "Maths/Vector3.h"

struct ProjectionViewUniform
{
	Matrix4 proj;
	Matrix4 view;
	Matrix4 model;
	Vector3 cameraLocation;
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
	uint32 type;
	Vector3 location;
	Vector3 direction;
	Color color;
};