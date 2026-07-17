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

public:
	Material* GetMaterial() const;

	void Render() override;

};