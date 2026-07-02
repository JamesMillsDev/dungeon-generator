#include "pch.h"
#include "FlyMovement.h"

#include "Gameplay/SimpleInput.h"
#include "Gameplay/Actors/Actor.h"
#include "Gameplay/Actors/Transform.h"

FlyMovement::FlyMovement(const float turnSpeed, const float moveSpeed)
	: m_theta{ 0 }, m_phi{ 0 }, m_turnSpeed{ Maths::Radians(turnSpeed) }, m_moveSpeed{ moveSpeed }
{}

void FlyMovement::BeginPlay()
{
	SimpleInput* input = SimpleInput::Instance();
	float mx = input->GetMouseX();
	float my = input->GetMouseY();
	m_lastMouse = { mx, my };
}

void FlyMovement::Tick()
{
	SimpleInput* input = SimpleInput::Instance();

	// Ignore the movement if we aren't holding right click
	if (!input->IsMouseButtonDown(MouseButtonRight))
	{
		return;
	}

	Transform* transform = Owner()->GetTransform();

	vec3 forward = transform->Forward();
	const vec3 right = transform->Right();
	constexpr vec3 up{ 0, 1, 0 };

	if (input->IsKeyDown(KeyW))
	{
		transform->location += forward * GameTime::DeltaTime() * m_moveSpeed;
	}

	if (input->IsKeyDown(KeyS))
	{
		transform->location -= forward * GameTime::DeltaTime() * m_moveSpeed;
	}

	if (input->IsKeyDown(KeyA))
	{
		transform->location += right * GameTime::DeltaTime() * m_moveSpeed;
	}

	if (input->IsKeyDown(KeyD))
	{
		transform->location -= right * GameTime::DeltaTime() * m_moveSpeed;
	}

	if (input->IsKeyDown(KeyQ))
	{
		transform->location += up * GameTime::DeltaTime() * m_moveSpeed;
	}

	if (input->IsKeyDown(KeyE))
	{
		transform->location -= up * GameTime::DeltaTime() * m_moveSpeed;
	}

	float mx = input->GetMouseX();
	float my = input->GetMouseY();

	m_theta += m_turnSpeed * (mx - m_lastMouse.x) * GameTime::DeltaTime();
	m_phi += m_turnSpeed * (my - m_lastMouse.y) * GameTime::DeltaTime();

	m_lastMouse = { mx, my };

	const float thetaR = glm::radians(m_theta);
	const float phiR = glm::radians(m_phi);

	transform->rotation = quat{ vec3{ phiR, thetaR, 0.f } };
}
