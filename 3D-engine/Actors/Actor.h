#pragma once

#include <functional>
#include <utility>
#include <vector>

#include "Components/IComponent.h"

class Transform;

using std::function;
using std::pair;
using std::vector;

using ComponentListChange = pair<IComponent*, function<void(IComponent*)>>;

class Actor
{
	friend class World;

private:
	Transform* m_transform;

	vector<IComponent*> m_components;
	vector<ComponentListChange> m_componentListChanges;

private:
	Actor();
	virtual ~Actor();

public:
	virtual void BeginPlay();

	virtual void Tick();
	virtual void Render();

	virtual void EndPlay();

	template<typename T, typename... ARGS>
	T* MakeComponent(ARGS... args);

	void DestroyComponent(IComponent*& component);

	Transform* GetTransform() const;

private:
	void ApplyComponentListChanges();

};

template <typename T, typename... ARGS>
T* Actor::MakeComponent(ARGS... args)
{
	static_assert(std::is_base_of_v<IComponent, T>, "T must derive from IComponent");

	T* newComp = new T{ std::forward<ARGS...>(args...) };
	m_componentListChanges.emplace_back(newComp, [this](IComponent* comp)
		{
			comp->BeginPlay();
			m_components.emplace_back(comp);
		});

	newComp->m_owner = this;
	return newComp;
}
