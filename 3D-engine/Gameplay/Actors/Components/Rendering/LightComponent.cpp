#include "pch.h"
#include "LightComponent.h"

LightComponent::LightComponent()
	: type{ EType::Directional }, intensity{ 1.f }, color{ Color::WHITE }
{

}