#include "pch.h"
#include "Actor.h"

#include "Transform.h"

Actor::Actor()
	: m_transform{ new Transform }
{
	m_transform->m_owner = this;
}

Actor::~Actor()
{
	for (IComponent* component : m_components)
	{
		DestroyComponent(component);
	}
	ApplyComponentListChanges();

	for (Transform* child : m_transform->Children())
	{
		delete child->Owner();
	}

	delete m_transform;
}

void Actor::BeginPlay()
{}

void Actor::Tick()
{}

void Actor::Render()
{}

void Actor::EndPlay()
{}

void Actor::DestroyComponent(IComponent*& component)
{
	m_componentListChanges.emplace_back(component, [this](IComponent* comp)
		{
			std::erase_if(m_components, [comp](IComponent* c)
				{
					return c == comp;
				});
			comp->EndPlay();
			delete comp;
			comp = nullptr;
		});
}

Transform* Actor::GetTransform() const
{
	return m_transform;
}

void Actor::ApplyComponentListChanges()
{
	for (const auto& [component, change] : m_componentListChanges)
	{
		change(component);
	}
	m_componentListChanges.clear();
}
