#pragma once

#include <functional>
#include <utility>
#include <vector>

#include "Actor.h"
#include "Transform.h"

using std::function;
using std::pair;
using std::vector;

using ActorLifetimeChange = pair<Actor*, function<void(Actor*)>>;

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

	void DestroyActor(Actor*& actor);

private:
	void Tick(Actor* actor = nullptr);
	void Render(Actor* actor = nullptr);

};

template <typename T, typename ... ARGS>
T* World::MakeActor(ARGS... args)
{
	static_assert(std::is_base_of_v<Actor, T>, "T must derive from Actor");

	T* newActor = new T{ args... };
	m_lifetimeChanges.emplace_back(newActor, [this](Actor* toSpawn)
		{
			toSpawn->GetTransform()->SetParent(m_root->GetTransform());
			toSpawn->GetTransform()->ApplyChildListChanges();

			toSpawn->BeginPlay();
			toSpawn->ApplyComponentListChanges();

			for (IComponent* component : toSpawn->m_components)
			{
				component->BeginPlay();
			}

		});

	return newActor;
}
