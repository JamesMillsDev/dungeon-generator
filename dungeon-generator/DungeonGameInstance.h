#pragma once

#include <Gameplay/GameInstance.h>

class Actor;

class DungeonGameInstance final : public GameInstance
{
private:
	Actor* m_actor;

public:
	DungeonGameInstance();

public:
	void Init() override;
	void Shutdown() override;
	void Tick() override;
	void Render() override;

};
