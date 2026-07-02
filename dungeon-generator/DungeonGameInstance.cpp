#include "DungeonGameInstance.h"

#include "Gameplay/Actors/Transform.h"
#include "Gameplay/Actors/World.h"
#include "Gameplay/Actors/Components/FlyMovement.h"
#include "Gameplay/Actors/Components/Rendering/CameraComponent.h"
#include "Gameplay/Actors/Components/Rendering/MeshComponent.h"

#include "Graphics/Rendering/Material.h"
#include "Graphics/Rendering/Mesh.h"
#include "Graphics/Rendering/Texture.h"

DungeonGameInstance::DungeonGameInstance() :
	m_meshActor{ nullptr }, m_cameraActor{ nullptr }, m_material{ nullptr }, m_mesh{ nullptr }
{}

void DungeonGameInstance::Init()
{
	m_cameraActor = GetWorld()->MakeActor<Actor>();
	m_cameraActor->MakeComponent<CameraComponent>(45.f, .1f, 100.f);
	m_cameraActor->GetTransform()->location += vec3{ 0.f, 2.f, -10.f };
	m_cameraActor->MakeComponent<FlyMovement>(180.f, 5.f);

	m_mesh = Mesh::MakeFromAssimp("Meshes/SM_Soulspear.fbx");
	m_material = new Material{ "Shaders/pbr" };
	m_material->baseColorMap = new Texture{ "Textures/T_Soulspear_B" }; 

	m_meshActor = GetWorld()->MakeActor<Actor>();
	m_meshActor->MakeComponent<MeshComponent>(m_mesh, m_material);
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