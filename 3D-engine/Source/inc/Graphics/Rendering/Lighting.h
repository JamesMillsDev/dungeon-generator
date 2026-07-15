#pragma once

#include "Maths/Alias.h"
#include "Maths/Color.h"

#include "Utility/Collections/TList.h"

constexpr uint8 MAX_LIGHT_COUNT = 8;

struct LightUniform
{
	vec4 location;
	vec4 direction;
	Color color;
	int32 type;
	int32 enabled;
};

struct SceneLightingUniform
{
	Color ambientColor;
	float ambientStrength;
};

class LightComponent;

class Lighting
{
private:
	SceneLightingUniform m_sceneLighting;
	TList<LightComponent*> m_lights;

public:
	Lighting();

public:
	void UpdateBuffers();
	void ShowWindow();

	void AddLight(LightComponent* light);
	void RemoveLight(LightComponent* light);

};