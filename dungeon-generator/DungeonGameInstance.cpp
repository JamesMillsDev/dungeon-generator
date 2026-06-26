#include "DungeonGameInstance.h"

#include "Gameplay/Actors/Transform.h"
#include "Gameplay/Actors/World.h"
#include "Gameplay/Actors/Components/Rendering/MeshComponent.h"

#include "Graphics/Rendering/Material.h"
#include "Graphics/Rendering/Mesh.h"

DungeonGameInstance::DungeonGameInstance() :
	m_actor{ nullptr }, m_material{ nullptr }, m_mesh{ nullptr }
{}

void DungeonGameInstance::Init()
{
	m_mesh = Mesh::MakeFromAssimp("Meshes/SM_Windmill.fbx");
	m_material = new Material{ "Shaders/pbr" };

	m_actor = GetWorld()->MakeActor<Actor>();
	m_actor->GetTransform()->AddRelativeScale({ 10.f, 10.f, 10.f });
	m_actor->MakeComponent<MeshComponent>(m_mesh, m_material);
}

void DungeonGameInstance::Shutdown()
{
	delete m_mesh;
	delete m_material;
}

void DungeonGameInstance::Tick()
{}

void DungeonGameInstance::Render()
{}