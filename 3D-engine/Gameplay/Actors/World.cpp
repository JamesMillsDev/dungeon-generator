#include "pch.h"
#include "World.h"

#include "Gameplay/Actors/Transform.h"

World::World()
	: m_root{ new Actor }
{ }

World::~World()
{
	delete m_root;
}

void World::DestroyActor(Actor* actor)
{
	m_lifetimeChanges.emplace_back([this, actor]
	{
			actor->GetTransform()->SetParent(nullptr);
			actor->GetTransform()->ApplyChildListChanges();

			actor->EndPlay();
			actor->ApplyComponentListChanges();

			for (IComponent* component : actor->m_components)
			{
				component->EndPlay();
			}

			delete actor;
		});
}

void World::Tick(Actor* actor)
{
	if (actor == nullptr)
	{
		actor = m_root;
		for (const ActorLifetimeChange& change : m_lifetimeChanges)
		{
			change();
		}
		m_lifetimeChanges.clear();
	}

	actor->m_transform->ApplyChildListChanges();
	actor->ApplyComponentListChanges();

	actor->Tick();
	for (IComponent* component : actor->m_components)
	{
		component->Tick();
	}

	for (Transform* child : actor->GetTransform()->Children())
	{
		Tick(child->Owner());
	}
}

void World::Render(Actor* actor)
{
	if (actor == nullptr)
	{
		actor = m_root;
	}

	if (actor != m_root)
	{
		actor->Render();
		for (IComponent* component : actor->m_components)
		{
			component->Render();
		}
	}

	for (Transform* child : actor->GetTransform()->Children())
	{
		Render(child->Owner());
	}
}