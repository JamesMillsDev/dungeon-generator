#pragma once

#include <Gameplay/GameInstance.h>

class Mesh;
class Material;
class Actor;

class DungeonGameInstance final : public GameInstance
{
private:
	Actor* m_meshActor;
	Actor* m_cameraActor;

	Material* m_material;
	Mesh* m_mesh;

public:
	DungeonGameInstance();

public:
	void Init() override;
	void Shutdown() override;
	void Tick() override;
	void Render() override;

};
