#include "DungeonGameInstance.h"

#include "FlyCamera.h"

#include "Gameplay/Actors/Transform.h"
#include "Gameplay/Actors/World.h"
#include "Gameplay/Actors/Components/Rendering/LightComponent.h"
#include "Gameplay/Actors/Components/Rendering/MeshComponent.h"

#include "Graphics/Rendering/Material.h"
#include "Graphics/Rendering/Mesh.h"
#include "Graphics/Rendering/Texture.h"

#include "ImGui/imgui.h"

DungeonGameInstance::DungeonGameInstance() :
	m_meshActor{ nullptr }, m_camera{ nullptr }, m_material{ nullptr }, m_mesh{ nullptr }
{}

void DungeonGameInstance::Init()
{
	m_camera = new FlyCamera{ 45.f, .1f, 100.f };
	m_camera->location = vec3{ 0.f, 2.f, -10.f };

	m_mesh = Mesh::MakeCube(); 
	m_material = new Material{ "Shaders/pbr" };
	m_material->AddTexture(Texture::LoadFromFile("Textures/T_Brick_B"));
	m_material->AddTexture(Texture::LoadFromFile("Textures/T_Brick_N")); 
	m_material->AddTexture(Texture::LoadFromFile("Textures/T_Brick_ORM")); 
	m_material->color = Color{ 1.f, 1.f, 1.f, 1.f };

	m_meshActor = GetWorld()->MakeActor<Actor>();
	m_meshActor->MakeComponent<MeshComponent>(m_mesh, m_material);

	vec3 lightPositions[] = 
	{
		vec3(-10.0f, -10.0f, 10.0f),
		vec3(10.0f,  -10.0f, 10.0f),
		vec3(-10.0f, 10.0f, 10.0f),
		vec3(10.0f, 10.0f, 10.0f),
	};
	vec3 lightDirections[] =
	{
		vec3(1.0f, 0.0f, 0.0f),
		vec3(0.0f, 1.0f, 0.0f),
		vec3(0.0f, 0.0f, 1.0f),
		vec3(0.0f, 0.0f, -1.0f),
	};
	Color lightColors[] = 
	{
		Color::WHITE,
		Color::WHITE,
		Color::WHITE,
		Color::WHITE
	};

	// temp 4 lights
	for (uint8 i = 0; i < 4; ++i)
	{
		Actor* lightActor = GetWorld()->MakeActor<Actor>();
		LightComponent* light = lightActor->MakeComponent<LightComponent>();
		lightActor->GetTransform()->location = lightPositions[i];
		lightActor->GetTransform()->rotation = glm::quatLookAt(lightDirections[i], { 0.f, -1.f, 0.f });
		light->type = LightComponent::EType::Point;
		light->color = lightColors[i];
		//light->intensity = 300.f;
	}
}

void DungeonGameInstance::Shutdown()
{
	delete m_camera;
	delete m_mesh;
	delete m_material;
}

void DungeonGameInstance::Tick()
{
	m_camera->Tick();
}

void DungeonGameInstance::Render()
{

}