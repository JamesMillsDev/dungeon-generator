#pragma once

#include "Gameplay/Actors/Components/IComponent.h"

class GraphicsPipeline;
class Mesh;

class MeshComponent final : public IComponent
{
private:
	Mesh* m_mesh;
	GraphicsPipeline* m_pipeline;

public:
	MeshComponent(Mesh* mesh, GraphicsPipeline* pipeline);
	~MeshComponent() override;

public:
	void BeginPlay() override;
	void Render() override;
	void EndPlay() override;

};