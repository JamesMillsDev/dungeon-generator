#pragma once

#include "Graphics/Vulkan/Uniforms.h"

class Window;

class Camera
{
	friend class Renderer;

public:
	float fovY;
	float nearPlane;
	float farPlane;

protected:
	Window* m_window;

private:
	bool m_isCurrent;

public:
	Camera(float fovY, float nearPlane, float farPlane);
	virtual ~Camera() = default;

public:
	virtual void GetPvm(ProjectionViewModelUniform& pvm) const;

	[[nodiscard]] bool IsCurrent() const;

};
