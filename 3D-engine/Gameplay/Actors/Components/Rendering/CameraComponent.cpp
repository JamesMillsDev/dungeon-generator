#include "pch.h"
#include "CameraComponent.h"

#include "Application.h"
#include "Gameplay/Actors/Actor.h"
#include "Gameplay/Actors/Transform.h"
#include "Graphics/Renderer.h"

CameraComponent::CameraComponent(const float fovY, const float nearPlane, const float farPlane)
	: fovY{ fovY }, nearPlane{ nearPlane }, farPlane{ farPlane }, m_window{ Application::GetWindow() }
{
	Renderer::SetCurrent(this);
}

void CameraComponent::GetPvm(ProjectionViewModelUniform& pvm) const
{
	const Transform* transform = Owner()->GetTransform();

	pvm.proj = glm::perspective(Maths::Radians(fovY), m_window->Aspect(), nearPlane, farPlane);
	pvm.view = transform->GlobalTransform();
	pvm.cameraLocation = transform->Location();
}

bool CameraComponent::IsCurrent() const
{
	return m_isCurrent;
}
