#include "pch.h"
#include "Transform.h"

Transform::Transform()
	: m_owner{ nullptr }, m_relativeTransform{ Matrix4::Identity() }, m_parent{ nullptr }
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

Matrix4 Transform::GlobalTransform() const
{
	return m_parent != nullptr ? m_relativeTransform * m_parent->GlobalTransform() : m_relativeTransform;
}

Vector3 Transform::Location() const
{
	return GlobalTransform().Translation();
}

Vector3 Transform::EulerAngles() const
{
	return GlobalTransform().EulerAngles();
}

Vector3 Transform::Scale() const
{
	return GlobalTransform().Scale();
}

Vector3 Transform::RelativeLocation() const
{
	return m_relativeTransform.Translation();
}

Vector3 Transform::RelativeEulerAngles() const
{
	return m_relativeTransform.EulerAngles();
}

Vector3 Transform::RelativeScale() const
{
	return m_relativeTransform.Scale();
}

void Transform::SetLocation(const Vector3& location)
{

}

void Transform::SetEulerAngles(const Vector3& euler)
{

}

void Transform::SetScale(const Vector3& scale)
{

}

void Transform::SetRelativeLocation(const Vector3& location)
{

}

void Transform::SetRelativeEulerAngles(const Vector3& euler)
{
	const Vector3 location = RelativeLocation();
	const Vector3 scale = RelativeScale();

	m_relativeTransform = Matrix4::MakeTranslate(location) * Matrix4::MakeRotate(euler) * Matrix4::MakeScale(scale);
}

void Transform::SetRelativeScale(const Vector3& scale)
{
	const Vector3 location = RelativeLocation();
	const Vector3 euler = RelativeEulerAngles();

	m_relativeTransform = Matrix4::MakeTranslate(location) * Matrix4::MakeRotate(euler) * Matrix4::MakeScale(scale);
}

void Transform::AddLocation(const Vector3& location)
{}

void Transform::AddEulerAngles(const Vector3& euler)
{}

void Transform::AddScale(const Vector3& scale)
{

}

void Transform::AddRelativeLocation(const Vector3& location)
{
	m_relativeTransform *= Matrix4::MakeTranslate(location);
}

void Transform::AddRelativeEulerAngles(const Vector3& euler)
{
	m_relativeTransform *= Matrix4::MakeRotate(euler);
}

void Transform::AddRelativeScale(const Vector3& scale)
{
	m_relativeTransform *= Matrix4::MakeScale(scale);
}

void Transform::ApplyChildListChanges()
{
	for (const TransformChildrenUpdate& change : m_childListUpdates)
	{
		change();
	}
	m_childListUpdates.clear();
}
