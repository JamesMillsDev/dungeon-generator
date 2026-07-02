#pragma once

#include <functional>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

class Actor;

using IterationFunc = std::function<void(class Transform*, int)>;
using glm::mat4;
using glm::quat;
using glm::vec3;

class Transform
{
	friend Actor;

public:
	vec3 location;
	quat rotation;
	vec3 scale;

	Transform* parent;
	Transform* nextSibling;
	Transform* previousSibling;
	Transform* lastChild;

private:
	Actor* m_owner;

private:
	Transform();
	~Transform();

public:
	[[nodiscard]] Actor* Owner() const;

	[[nodiscard]] mat4 LocalToWorld() const;
	[[nodiscard]] mat4 WorldToLocal() const;

	[[nodiscard]] vec3 Right() const;
	[[nodiscard]] vec3 Up() const;
	[[nodiscard]] vec3 Forward() const;

	void SetParent(Transform* newParent, Transform* before = nullptr);
	void ForEachChild(const IterationFunc& iteration) const;

private:
	[[nodiscard]] mat4 LocalToParent() const;
	[[nodiscard]] mat4 ParentToLocal() const;

private:
	void ValidatePointers() const;

};
