#include "DungeonGameInstance.h"

#include "FlyCamera.h"
#include "GameTime.h"

#include "Gameplay/Actors/Transform.h"
#include "Gameplay/Actors/World.h"
#include "Gameplay/Actors/Components/Rendering/LightComponent.h"
#include "Gameplay/Actors/Components/Rendering/MeshComponent.h"

#include "Graphics/Rendering/Lighting.h"
#include "Graphics/Rendering/Material.h"
#include "Graphics/Rendering/Mesh.h"
#include "Graphics/Rendering/Texture.h"

#include "ImGui/imgui.h"

#include "Maths/Maths.h"

namespace
{
	TArray<Actor*, MAX_LIGHT_COUNT> lights;
	TArray<Material*, MAX_LIGHT_COUNT> lightMaterials;
	mat4 rotationMatrix(1.f);
}
constexpr float ANGLE_STEP = 360.f / MAX_LIGHT_COUNT;

DungeonGameInstance::DungeonGameInstance()
	: m_meshActor{ nullptr }, m_camera{ nullptr }, m_material{ nullptr },
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
	m_material->showDebugWindow = true;
	
	m_meshActor = GetWorld()->MakeActor<Actor>();
	m_meshActor->MakeComponent<MeshComponent>(m_mesh, m_material);

	m_cubeMesh = Mesh::MakeCube();

	float offset = .00001f;
	for (uint8 i = 0; i < MAX_LIGHT_COUNT; ++i)
	{
		lightMaterials[i] = new Material{ "Shaders/unlit" };
		lightMaterials[i]->color = Color::HSVToRGB(offset, 1.f, 1.f);

		Actor* lightActor = GetWorld()->MakeActor<Actor>();
		LightComponent* light = lightActor->MakeComponent<LightComponent>();
		lightActor->MakeComponent<MeshComponent>(m_cubeMesh, lightMaterials[i]);
		lightActor->GetTransform()->scale = vec3{ .25f };

		light->type = LightComponent::EType::Point;
		light->color = lightMaterials[i]->color;

		lights[i] = lightActor;
		offset += ANGLE_STEP;
	}
}

void DungeonGameInstance::Shutdown()
{
	delete m_camera;
	delete m_mesh;
	delete m_material;

	delete m_cubeMesh;
	for (Material* material : lightMaterials)
	{
		delete material;
	}
}

void DungeonGameInstance::Tick()
{
	m_camera->Tick();

	float offset = 0;
	for (Actor* light : lights)
	{
		rotationMatrix = glm::rotate(mat4{ 1.f }, GameTime::Time(), vec3{ 0.f, 1.f, 0.f });
		rotationMatrix = glm::rotate(rotationMatrix, Maths::Radians(offset), vec3{ 0.f, 1.f, 0.f });

		offset += ANGLE_STEP;
		vec3 forward = rotationMatrix * vec4{ 0.f, 0.f, 1.f, 0.f };

		light->GetTransform()->location = forward * 5.f + vec3{ 0.f, 2.5f, 0.f };
	}
}

void DungeonGameInstance::Render()
{

}