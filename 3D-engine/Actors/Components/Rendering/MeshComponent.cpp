#include "pch.h"
#include "MeshComponent.h"

#include "Renderer.h"

#include "Rendering/Mesh.h"

MeshComponent::MeshComponent(Mesh* mesh)
	: m_mesh{ mesh }
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
	Renderer::GetInstance()->Render(m_mesh);
}

void MeshComponent::EndPlay()
{
	Renderer::Unload(m_mesh);
}
