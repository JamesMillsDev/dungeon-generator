#pragma once

#include "IComponent.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

using glm::vec2;
using glm::vec3;

class FlyMovement : public IComponent
{
private:
	float m_theta;
	float m_phi;

	float m_turnSpeed;
	float m_moveSpeed;

	vec2 m_lastMouse;

public:
	FlyMovement(float turnSpeed, float moveSpeed);

public:
	void BeginPlay() override;
	void Tick() override;

};