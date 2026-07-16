#include "Graphics/Rendering/SceneCamera.h"

#include "Gameplay/Actors/Transform.h"

SceneCamera::SceneCamera(const float fov, const float near, const float far)
	: Camera{ fov, near, far }, m_transform{ nullptr }
{
	
}

void SceneCamera::SetTransform(Transform* transform)
{
	m_transform = transform;
}

void SceneCamera::GetPvm(ProjectionViewUniform& pvm) const
{
	Camera::GetPvm(pvm);

	const mat4 view = m_transform->LocalToWorld();

	pvm.view = glm::scale(view, { 1.f, -1.f, 1.f });
	pvm.cameraLocation = m_transform->location;
}