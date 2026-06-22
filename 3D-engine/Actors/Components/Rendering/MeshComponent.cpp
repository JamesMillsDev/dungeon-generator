#include "pch.h"
#include "MeshComponent.h"

#include "Renderer.h"

#include "Rendering/Mesh.h"

MeshComponent::MeshComponent(Mesh* mesh, GraphicsPipeline* pipeline)
	: m_mesh{ mesh }, m_pipeline{ pipeline }
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
	Renderer::GetInstance()->Render(m_mesh, m_pipeline);
}

void MeshComponent::EndPlay()
{
	Renderer::Unload(m_mesh);
}
