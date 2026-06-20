#include "pch.h"
#include "World.h"

#include "Transform.h"

World::World()
	: m_root{ new Actor }
{ }

World::~World()
{
	delete m_root;
}

void World::DestroyActor(Actor*& actor)
{
	m_lifetimeChanges.emplace_back(actor, [this](Actor* toDestroy)
		{
			toDestroy->GetTransform()->SetParent(nullptr);
			toDestroy->GetTransform()->ApplyChildListChanges();

			toDestroy->EndPlay();
			toDestroy->ApplyComponentListChanges();

			for (IComponent* component : toDestroy->m_components)
			{
				component->EndPlay();
			}

			delete toDestroy;
			toDestroy = nullptr;
		});
}

void World::Tick(Actor* actor)
{
	if (actor == nullptr)
	{
		actor = m_root;
		for (const auto& [actor, change] : m_lifetimeChanges)
		{
			change(actor);
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