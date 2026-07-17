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

DungeonGameInstance::DungeonGameInstance()
	: m_meshActor{ nullptr }, m_camera{ nullptr }, m_material{ nullptr }, m_cubeMaterial{ },
	m_mesh{ nullptr }, m_cubeMesh{ nullptr }
{}

void DungeonGameInstance::Init()
{
	m_camera = new FlyCamera{ 45.f, .1f, 100.f };
	m_camera->location = vec3{ 0.f, 2.f, -10.f };

	m_mesh = Mesh::MakeFromAssimp("Meshes/SM_Soulspear.fbx");
	m_material = new Material{ "Shaders/pbr" };
	m_material->SetTexture(BASE_COLOR_MAP_NAME, Texture::LoadFromFile("Textures/T_Soulspear_B"));
	m_material->SetTexture(NORMAL_MAP_NAME, Texture::LoadFromFile("Textures/T_Soulspear_N"));
	m_material->SetTexture(ORM_MAP_NAME, Texture::LoadFromFile("Textures/T_Soulspear_ORM"));
	m_material->color = Color{ 1.f, 1.f, 1.f, 1.f };

	m_meshActor = GetWorld()->MakeActor<Actor>();
	m_meshActor->MakeComponent<MeshComponent>(m_mesh, m_material);

	constexpr vec3 lightPositions[] =
	{
		vec3{ -5.f, 0.f, -5.f },
		vec3{ -5.f, 0.f,  5.f },
		vec3{  5.f, 0.f, -5.f },
		vec3{  5.f, 0.f,  5.f },
	};

	const Color lightColors[] =
	{
		Color::WHITE,
		Color::WHITE,
		Color::WHITE,
		Color::WHITE
	};

	m_cubeMesh = Mesh::MakeCube();

	// temp 4 lights
	for (uint8 i = 0; i < 4; ++i)
	{
		m_cubeMaterial[i] = new Material{ "Shaders/unlit" };
		m_cubeMaterial[i]->color = lightColors[i];

		Actor* lightActor = GetWorld()->MakeActor<Actor>();
		LightComponent* light = lightActor->MakeComponent<LightComponent>();
		lightActor->MakeComponent<MeshComponent>(m_cubeMesh, m_cubeMaterial[i]);
		lightActor->GetTransform()->location = lightPositions[i];
		lightActor->GetTransform()->scale = vec3{ .25f };

		light->type = LightComponent::EType::Point;
		light->color = lightColors[i];
		light->intensity = 300.f;
	}
}

void DungeonGameInstance::Shutdown()
{
	delete m_camera;
	delete m_mesh;
	delete m_material;

	delete m_cubeMesh;
	for (const Material* mat : m_cubeMaterial)
	{
		delete mat;
	}
}

void DungeonGameInstance::Tick()
{
	m_camera->Tick();
}

void DungeonGameInstance::Render()
{

}