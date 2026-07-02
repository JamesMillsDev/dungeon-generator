// TODO: finish this

#include "pch.h"
#include "Transform.h"

#include <glm/gtx/matrix_decompose.hpp>

Transform::Transform()
	: m_owner{ nullptr }, m_transform{ mat4{ 1.f } }, m_parent{ nullptr }
{}

Transform::~Transform()
{
	if (m_parent != nullptr)
	{
		SetParent(nullptr);
		ApplyChildListChanges();
	}

	m_owner = nullptr;
	for (const Transform* child : m_children)
	{
		delete child;
	}
	m_children.clear();
}

Actor* Transform::Owner() const
{
	return m_owner;
}

Transform* Transform::Parent() const
{
	return m_parent;
}

vector<Transform*> Transform::Children() const
{
	return m_children;
}

void Transform::SetParent(Transform* parent)
{
	if (parent == nullptr && m_parent != nullptr)
	{
		m_childListUpdates.emplace_back([this]
			{
				if (m_parent != nullptr)
				{
					std::erase_if(m_parent->m_children, [this](const Transform* child)
						{
							return child == this;
						});

					m_parent = nullptr;
				}
			});

		return;
	}

	if (m_parent != parent)
	{
		m_childListUpdates.emplace_back([this, parent]
			{
				if (m_parent != nullptr)
				{
					std::erase_if(m_parent->m_children, [this](const Transform* child)
						{
							return child == this;
						});

					m_parent = nullptr;
				}

				m_parent = parent;
				m_parent->m_children.emplace_back(this);
			});
	}
}

mat4 Transform::GlobalTransform() const
{
	return m_parent != nullptr ? m_transform * m_parent->GlobalTransform() : m_transform;
}

vec3 Transform::Location() const
{
	vec3 translation;
	quat rotation;
	vec3 scale;
	vec3 euler;

	DecomposeGlobal(translation, rotation, scale, euler);

	return translation;
}

quat Transform::Rotation() const
{
	vec3 translation;
	quat rotation;
	vec3 scale;
	vec3 euler;

	DecomposeGlobal(translation, rotation, scale, euler);

	return rotation;
}

vec3 Transform::EulerAngles() const
{
	vec3 translation;
	quat rotation;
	vec3 scale;
	vec3 euler;

	DecomposeGlobal(translation, rotation, scale, euler);

	return euler;
}

vec3 Transform::Scale() const
{
	vec3 translation;
	quat rotation;
	vec3 scale;
	vec3 euler;

	DecomposeGlobal(translation, rotation, scale, euler);

	return scale;
}

vec3 Transform::RelativeLocation() const
{
	vec3 translation;
	quat rotation;
	vec3 scale;
	vec3 euler;

	DecomposeLocal(translation, rotation, scale, euler);

	return translation;
}

vec3 Transform::RelativeEulerAngles() const
{
	vec3 translation;
	quat rotation;
	vec3 scale;
	vec3 euler;

	DecomposeLocal(translation, rotation, scale, euler);

	return euler;
}

vec3 Transform::RelativeScale() const
{
	vec3 translation;
	quat rotation;
	vec3 scale;
	vec3 euler;

	DecomposeLocal(translation, rotation, scale, euler);

	return scale;
}

void Transform::SetLocation(const vec3& location)
{

}

void Transform::SetRotation(const quat& rotation)
{}

void Transform::SetEulerAngles(const vec3& euler)
{

}

void Transform::SetScale(const vec3& scale)
{

}

void Transform::SetRelativeLocation(const vec3& location)
{
	vec3 translation;
	quat rotation;
	vec3 scale;
	vec3 euler;

	DecomposeLocal(translation, rotation, scale, euler);

	m_transform = glm::translate(m_transform, location) * mat4(rotation) * glm::scale(scale);
}

void Transform::SetRelativeRotation(const quat& rotation)
{
	vec3 translation;
	quat rotation1;
	vec3 scale;
	vec3 euler1;

	DecomposeLocal(translation, rotation1, scale, euler1);

	m_transform = glm::translate(m_transform, translation) * mat4(rotation) * glm::scale(scale);
}

void Transform::SetRelativeEulerAngles(const vec3& euler)
{
	vec3 translation;
	quat rotation;
	vec3 scale;
	vec3 euler1;

	DecomposeLocal(translation, rotation, scale, euler1);

	m_transform = glm::translate(m_transform, translation) * mat4(quat(euler)) * glm::scale(scale);
}

void Transform::SetRelativeScale(const vec3& scale)
{
	vec3 translation;
	quat rotation;
	vec3 scale1;
	vec3 euler;

	DecomposeLocal(translation, rotation, scale1, euler);

	m_transform = glm::translate(m_transform, translation) * mat4(quat(euler)) * glm::scale(scale);
}

void Transform::AddLocation(const vec3& location)
{

}

void Transform::AddRotation(const quat& rotation)
{}

void Transform::AddEulerAngles(const vec3& euler)
{}

void Transform::AddScale(const vec3& scale)
{

}

void Transform::AddRelativeLocation(const vec3& location)
{
	m_transform = glm::translate(m_transform, { location.x, -location.y, location.z });
}

void Transform::AddRelativeRotation(const quat& rotation)
{
	m_transform *= mat4(rotation);
}

void Transform::AddRelativeEulerAngles(const vec3& euler)
{
	m_transform *= mat4(quat(euler));
}

void Transform::AddRelativeScale(const vec3& scale)
{
	m_transform = glm::scale(m_transform, scale);
}

void Transform::ApplyChildListChanges()
{
	for (const TransformChildrenUpdate& change : m_childListUpdates)
	{
		change();
	}
	m_childListUpdates.clear();
}

void Transform::DecomposeLocal(vec3& location, quat& rotation, vec3& scale, vec3& euler) const
{
	vec3 skew;
	vec4 perspective;

	glm::decompose(m_transform, scale, rotation, location, skew, perspective);

	euler = glm::eulerAngles(rotation);
}

void Transform::DecomposeGlobal(vec3& location, quat& rotation, vec3& scale, vec3& euler) const
{
	vec3 skew;
	vec4 perspective;

	glm::decompose(GlobalTransform(), scale, rotation, location, skew, perspective);

	euler = glm::eulerAngles(rotation);
}
