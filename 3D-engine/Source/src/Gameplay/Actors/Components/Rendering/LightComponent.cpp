#include "Gameplay/Actors/Components/Rendering/LightComponent.h"

#include "Gameplay/Actors/Actor.h"
#include "Gameplay/Actors/World.h"

#include "Graphics/Rendering/Lighting.h"

LightComponent::LightComponent()
	: type{ EType::Directional }, intensity{ 1.f }, color{ Color::WHITE }
{

}

void LightComponent::BeginPlay()
{
	Owner()->GetWorld()->GetLighting()->AddLight(this);
}

void LightComponent::EndPlay()
{
	Owner()->GetWorld()->GetLighting()->RemoveLight(this);
}