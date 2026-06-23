#pragma once

#include <functional>
#include <utility>
#include <vector>

#include "Gameplay/Actors/Actor.h"

using std::function;
using std::pair;
using std::vector;

using ActorLifetimeChange = function<void()>;

class World
{
	friend class Application;
	friend class GameInstance;

private:
	Actor* m_root;

	vector<ActorLifetimeChange> m_lifetimeChanges;

private:
	World();
	~World();

public:
	template<typename T, typename... ARGS>
	T* MakeActor(ARGS... args);

	void DestroyActor(Actor* actor);

private:
	void Tick(Actor* actor = nullptr);
	void Render(Actor* actor = nullptr);

};

template <typename T, typename ... ARGS>
T* World::MakeActor(ARGS... args)
{
	static_assert(std::is_base_of_v<Actor, T>, "T must derive from Actor");

	T* actor = new T{ args... };
	m_lifetimeChanges.emplace_back([this, actor]()
		{
			actor->GetTransform()->SetParent(m_root->GetTransform());
			actor->GetTransform()->ApplyChildListChanges();

			actor->BeginPlay();
			actor->ApplyComponentListChanges();

		});

	return actor;
}
