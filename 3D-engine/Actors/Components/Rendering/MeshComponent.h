#pragma once

#include "Actors/Components/IComponent.h"

class Mesh;

class MeshComponent final : public IComponent
{
private:
	Mesh* m_mesh;

public:
	explicit MeshComponent(Mesh* mesh);
	~MeshComponent() override;

public:
	void BeginPlay() override;
	void Render() override;
	void EndPlay() override;

};