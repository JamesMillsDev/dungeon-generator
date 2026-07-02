#pragma once

#include "Gameplay/Actors/Components/IComponent.h"
#include "Graphics/Vulkan/Uniforms.h"

class Window;

class CameraComponent : public IComponent
{
	friend class Renderer;

public:
	float fovY;
	float nearPlane;
	float farPlane;

private:
	Window* m_window;
	bool m_isCurrent;

public:
	CameraComponent(float fovY, float nearPlane, float farPlane);

public:
	void GetPvm(ProjectionViewModelUniform& pvm) const;

	[[nodiscard]] bool IsCurrent() const;

};
