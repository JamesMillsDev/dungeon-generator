#pragma once

#include "Maths/Alias.h"
#include "Maths/Color.h"
#include "Gameplay/Actors/Components/IComponent.h"

class LightComponent : public IComponent
{
public:
	enum class EType : uint8
	{
		Directional,
		Point,
		Spot
	};

public:
	EType type;
	float intensity;
	Color color;

public:
	LightComponent();

};