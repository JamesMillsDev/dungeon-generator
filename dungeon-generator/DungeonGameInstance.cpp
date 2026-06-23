#include "DungeonGameInstance.h"

#include "Gameplay/Actors/World.h"
#include "Gameplay/Actors/Components/Rendering/MeshComponent.h"

#include "Graphics/Renderer.h"
#include "Graphics/Rendering/GraphicsPipeline.h"
#include "Graphics/Rendering/Mesh.h"

DungeonGameInstance::DungeonGameInstance()
	: m_actor{ nullptr }, m_pipeline{ nullptr }
{

}

void DungeonGameInstance::Init()
{
	m_pipeline = m_renderer->CreatePipeline("Triangle");

	Mesh* mesh = Mesh::MakeQuad();

	m_actor = GetWorld()->MakeActor<Actor>();
	m_actor->MakeComponent<MeshComponent>(mesh, m_pipeline);
}

void DungeonGameInstance::Shutdown()
{}

void DungeonGameInstance::Tick()
{}

void DungeonGameInstance::Render()
{}