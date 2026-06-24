#include "DungeonGameInstance.h"

#include "Gameplay/Actors/Transform.h"
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
	m_actor->GetTransform()->AddRelativeScale({ 10.f, 10.f, 10.f });
	m_actor->MakeComponent<MeshComponent>(
		Mesh::Load("Meshes/SM_Windmill.fbx"), m_renderer->CreateMaterial("Triangle", EMaterialPass::Opaque, 1)
	);
}

void DungeonGameInstance::Shutdown()
{}

void DungeonGameInstance::Tick()
{}

void DungeonGameInstance::Render()
{}