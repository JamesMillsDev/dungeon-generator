#pragma once

#include "Maths/Matrix4.h"

struct MvpBufferObject
{
public:
	Matrix4 model;
	Matrix4 view;
	Matrix4 proj;

};