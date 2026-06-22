#pragma once

#include <GameInstance.h>

class GraphicsPipeline;
class Actor;

class DungeonGameInstance final : public GameInstance
{
private:
	Actor* m_actor;
	GraphicsPipeline* m_pipeline;

public:
	DungeonGameInstance();

public:
	void Init() override;
	void Shutdown() override;
	void Tick() override;
	void Render() override;

};
