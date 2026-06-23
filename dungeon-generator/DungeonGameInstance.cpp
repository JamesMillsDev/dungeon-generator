#include "DungeonGameInstance.h"

#include "Gameplay/Actors/World.h"
#include "Gameplay/Actors/Components/Rendering/MeshComponent.h"

#include "Graphics/Renderer.h"
#include "Graphics/Rendering/GraphicsPipeline.h"
#include "Graphics/Rendering/Mesh.h"

DungeonGameInstance::DungeonGameInstance()
	: m_actor{ nullptr }
{

}

void DungeonGameInstance::Init()
{
	m_actor = GetWorld()->MakeActor<Actor>();
	m_actor->MakeComponent<MeshComponent>(
		Mesh::MakeQuad(), m_renderer->CreatePipeline("Triangle")
	);
}

void DungeonGameInstance::Shutdown()
{}

void DungeonGameInstance::Tick()
{}

void DungeonGameInstance::Render()
{}