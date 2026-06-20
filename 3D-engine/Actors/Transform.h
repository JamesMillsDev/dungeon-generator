#pragma once

#include <functional>
#include <vector>

#include "Maths/Matrix4.h"
#include "Maths/Vector3.h"

class Actor;

using std::function;
using std::pair;
using std::vector;

using TransformChildrenUpdate = function<void()>;

class Transform
{
	friend class Actor;
	friend class World;

private:
	Actor* m_owner;

	Matrix4 m_relativeTransform;

	Transform* m_parent;
	vector<Transform*> m_children;

	vector<TransformChildrenUpdate> m_childListUpdates;

private:
	Transform();
	~Transform();

public:
	Actor* Owner() const;

	Transform* Parent() const;
	vector<Transform*> Children() const;

	void SetParent(Transform* parent);
	Matrix4 GlobalTransform() const;

	[[nodiscard]] Vector3 Location() const;
	[[nodiscard]] Vector3 EulerAngles() const;
	[[nodiscard]] Vector3 Scale() const;

	[[nodiscard]] Vector3 RelativeLocation() const;
	[[nodiscard]] Vector3 RelativeEulerAngles() const;
	[[nodiscard]] Vector3 RelativeScale() const;

	void SetLocation(const Vector3& location);
	void SetEulerAngles(const Vector3& euler);
	void SetScale(const Vector3& scale);

	void SetRelativeLocation(const Vector3& location);
	void SetRelativeEulerAngles(const Vector3& euler);
	void SetRelativeScale(const Vector3& scale);

	void AddLocation(const Vector3& location);
	void AddEulerAngles(const Vector3& euler);
	void AddScale(const Vector3& scale);

	void AddRelativeLocation(const Vector3& location);
	void AddRelativeEulerAngles(const Vector3& euler);
	void AddRelativeScale(const Vector3& scale);

private:
	void ApplyChildListChanges();

};
