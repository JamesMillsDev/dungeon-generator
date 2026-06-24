#pragma once

#include "Gameplay/Actors/Components/IComponent.h"

class Material;
class Mesh;

class MeshComponent final : public IComponent
{
private:
	Mesh* m_mesh;
	Material* m_material;

public:
	MeshComponent(Mesh* mesh, Material* material);
	~MeshComponent() override;

public:
	void BeginPlay() override;
	void Render() override;
	void EndPlay() override;

};