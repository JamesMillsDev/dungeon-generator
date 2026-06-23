#include "pch.h"
#include "GameInstance.h"

#include "Actors/World.h"

GameInstance::GameInstance()
// TODO: Support levels instead of worlds
	: m_renderer{ nullptr }, m_world{ new World }
{
	
}

GameInstance::~GameInstance() = default;

World* GameInstance::GetWorld() const
{
	return m_world;
}
