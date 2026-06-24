#include "pch.h"
#include "MeshComponent.h"

#include "Gameplay/Actors/Actor.h"
#include "Gameplay/Actors/Transform.h"

#include "Graphics/Renderer.h"
#include "Graphics/Rendering/Material.h"

#include "Graphics/Rendering/Mesh.h"

MeshComponent::MeshComponent(Mesh* mesh, Material* material)
	: m_mesh{ mesh }, m_material{ material }
{
	
}

MeshComponent::~MeshComponent()
{
	delete m_mesh;
}

void MeshComponent::BeginPlay()
{
	Renderer::Load(m_mesh);
}

void MeshComponent::Render()
{
	Renderer::GetInstance()->Render(m_mesh, m_material, Owner()->GetTransform()->GlobalTransform());
}

void MeshComponent::EndPlay()
{
	delete m_material;
	m_material = nullptr;

	Renderer::Unload(m_mesh);
}
