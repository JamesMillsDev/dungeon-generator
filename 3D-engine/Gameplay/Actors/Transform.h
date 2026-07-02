#pragma once

#include <functional>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>

class Actor;

using std::function;
using std::pair;
using std::vector;

using TransformChildrenUpdate = function<void()>;

using glm::mat4;
using glm::quat;
using glm::vec3;

class Transform
{
	friend class Actor;
	friend class World;

private:
	Actor* m_owner;

	mat4 m_transform;

	Transform* m_parent;
	vector<Transform*> m_children;

	vector<TransformChildrenUpdate> m_childListUpdates;

private:
	Transform();
	~Transform();

public:
	[[nodiscard]] Actor* Owner() const;

	[[nodiscard]] Transform* Parent() const;
	[[nodiscard]] vector<Transform*> Children() const;

	void SetParent(Transform* parent);
	[[nodiscard]] mat4 GlobalTransform() const;

	[[nodiscard]] vec3 Location() const;
	[[nodiscard]] quat Rotation() const;
	[[nodiscard]] vec3 EulerAngles() const;
	[[nodiscard]] vec3 Scale() const;

	[[nodiscard]] vec3 RelativeLocation() const;
	[[nodiscard]] vec3 RelativeEulerAngles() const;
	[[nodiscard]] vec3 RelativeScale() const;

	void SetLocation(const vec3& location);
	void SetRotation(const quat& rotation);
	void SetEulerAngles(const vec3& euler);
	void SetScale(const vec3& scale);

	void SetRelativeLocation(const vec3& location);
	void SetRelativeRotation(const quat& rotation);
	void SetRelativeEulerAngles(const vec3& euler);
	void SetRelativeScale(const vec3& scale);

	void AddLocation(const vec3& location);
	void AddRotation(const quat& rotation);
	void AddEulerAngles(const vec3& euler);
	void AddScale(const vec3& scale);

	void AddRelativeLocation(const vec3& location);
	void AddRelativeRotation(const quat& rotation);
	void AddRelativeEulerAngles(const vec3& euler);
	void AddRelativeScale(const vec3& scale);

private:
	void ApplyChildListChanges();

	void DecomposeLocal(vec3& location, quat& rotation, vec3& scale, vec3& euler) const;
	void DecomposeGlobal(vec3& location, quat& rotation, vec3& scale, vec3& euler) const;

};
