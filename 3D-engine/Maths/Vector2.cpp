#include "pch.h"
#include "Vector2.h"

#include "Maths.h"

#include <cassert>

#include "Vector3.h"
#include "Vector4.h"

constexpr float EPSILON_NORMAL_SQRT = 1e-15f;

const Vector2 Vector2::ONE = Vector2{ 1.f };
const Vector2 Vector2::HALF = Vector2{ .5f };
const Vector2 Vector2::ZERO = Vector2{ 0.f };
const Vector2 Vector2::UP = Vector2{ 0.f, 1.f };
const Vector2 Vector2::DOWN = Vector2{ 0.f, -1.f };
const Vector2 Vector2::RIGHT = Vector2{ 1.f, 0.f };
const Vector2 Vector2::LEFT = Vector2{ -1.f, 0.f };

Vector2 Vector2::Lerp(const Vector2& a, const Vector2& b, const float t)
{
	return a + (b - a) * Maths::Clamp01(t);
}

Vector2 Vector2::LerpUnclamped(const Vector2& a, const Vector2& b, const float t)
{
	return a + (b - a) * t;
}

Vector2 Vector2::MoveTowards(const Vector2& current, const Vector2& target, float maxDistanceDelta)
{
	const float toVectorX = target.x - current.x;
	const float toVectorY = target.y - current.y;

	const float sqrDist = toVectorX * toVectorX + toVectorY * toVectorY;

	if (Maths::IsNearZero(sqrDist) || (maxDistanceDelta >= 0 && sqrDist <= maxDistanceDelta * maxDistanceDelta))
	{
		return target;
	}

	const float dist = Maths::Sqrt(sqrDist);

	return Vector2
	{
		current.x + toVectorX / dist * maxDistanceDelta,
		current.y + toVectorY / dist * maxDistanceDelta,
	};
}

Vector2 Vector2::Reflect(const Vector2& inDirection, const Vector2& inNormal)
{
	const float factor = -2.f * Dot(inNormal, inDirection);
	return Vector2
	{
		factor * inNormal.x + inDirection.x,
		factor * inNormal.y + inDirection.y
	};
}

Vector2 Vector2::Perpendicular(const Vector2& vec)
{
	return Vector2{ -vec.y, vec.x };
}

float Vector2::Dot(const Vector2& a, const Vector2& b)
{
	return a.x * b.x + a.y * b.y;
}

float Vector2::AngleFromTo(const Vector2& from, const Vector2& to)
{
	const float denominator = Maths::Sqrt(from.MagnitudeSqr() * to.MagnitudeSqr());
	if (denominator < EPSILON_NORMAL_SQRT)
	{
		return 0.f;
	}

	const float dot = Maths::Clamp(Dot(from, to) / denominator, -1.f, 1.f);
	return Maths::Degrees(Maths::ACos(dot));
}

float Vector2::SignedAngleFromTo(const Vector2& from, const Vector2& to)
{
	const float unsignedAngle = AngleFromTo(from, to);
	const float sign = Maths::Sign(from.x * to.y - from.y * to.x);

	return unsignedAngle * sign;
}

float Vector2::Distance(const Vector2& a, const Vector2& b)
{
	const float diffX = a.x - b.x;
	const float diffY = a.y - b.y;

	return Maths::Sqrt(diffX * diffX + diffY * diffY);
}

Vector2 Vector2::ClampMagnitude(const Vector2& vec, float maxLength)
{
	const float sqrMag = vec.MagnitudeSqr();
	if (sqrMag > maxLength * maxLength)
	{
		const float mag = Maths::Sqrt(sqrMag);

		const float normX = vec.x / mag;
		const float normY = vec.y / mag;

		return Vector2
		{
			normX * maxLength,
			normY * maxLength
		};
	}

	return vec;
}

Vector2::Vector2()
	: x{ 0.f }, y{ 0.f }
{}

Vector2::Vector2(const float scalar)
	: x{ scalar }, y{ scalar }
{}

Vector2::Vector2(const float x, const float y)
	: x{ x }, y{ y }
{}

Vector2::Vector2(const vec2& vec)
	: x{ vec.x }, y{ vec.y }
{}

Vector2::Vector2(const vec3& vec)
	: x{ vec.x }, y{ vec.y }
{}

Vector2::Vector2(const vec4& vec)
	: x{ vec.x }, y{ vec.y }
{}

Vector2::Vector2(const Vector3& vec)
	: x{ vec.x }, y{ vec.y }
{}

Vector2::Vector2(const Vector4& vec)
	: x{ vec.x }, y{ vec.y }
{

}

Vector2::Vector2(const Vector2& rhs) = default;

void Vector2::Normalise()
{
	const float mag = Magnitude();
	if (Maths::IsNearZero(mag))
	{
		x = 0.f;
		y = 0.f;
	}
	else
	{
		x /= mag;
		y /= mag;
	}
}

Vector2 Vector2::Normalised() const
{
	const float mag = Magnitude();

	return Maths::IsNearZero(mag) ? Vector2{ 0.f, 0.f } : Vector2{ x / mag, y / mag };
}

float Vector2::Magnitude() const
{
	return Maths::Sqrt(MagnitudeSqr());
}

float Vector2::MagnitudeSqr() const
{
	return Dot(*this, *this);
}

Vector2::operator vec2() const
{
	return vec2{ x, y };
}

Vector2::operator vec3() const
{
	return vec3{ x, y, 0.f };
}

Vector2::operator vec4() const
{
	return vec4{ x, y, 0.f, 0.f };
}

Vector2::operator Vector3() const
{
	return Vector3{ x, y, 0.f };
}

Vector2::operator Vector4() const
{
	return Vector4{ x, y, 0.f, 0.f };
}

Vector2& Vector2::operator=(const Vector2& rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	x = rhs.x;
	y = rhs.y;

	return *this;
}

bool Vector2::operator==(const Vector2& rhs) const
{
	if (this == &rhs)
	{
		return true;
	}

	return Maths::Approx(x, rhs.x) && Maths::Approx(y, rhs.y);
}

bool Vector2::operator!=(const Vector2& rhs) const
{
	if (this == &rhs)
	{
		return false;
	}

	return !Maths::Approx(x, rhs.x) || !Maths::Approx(y, rhs.y);
}

Vector2 Vector2::operator+(const Vector2& rhs) const
{
	return { x + rhs.x, y + rhs.y };
}

Vector2& Vector2::operator+=(const Vector2& rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	x += rhs.x;
	y += rhs.y;

	return *this;
}

Vector2 Vector2::operator-(const Vector2& rhs) const
{
	return { x - rhs.x, y - rhs.y };
}

Vector2& Vector2::operator-=(const Vector2& rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	x -= rhs.x;
	y -= rhs.y;

	return *this;
}

Vector2 Vector2::operator*(const Vector2& rhs) const
{
	return { x * rhs.x, y * rhs.y };
}

Vector2& Vector2::operator*=(const Vector2& rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	x *= rhs.x;
	y *= rhs.y;

	return *this;
}

Vector2 Vector2::operator*(const float rhs) const
{
	return { x * rhs, y * rhs };
}

Vector2& Vector2::operator*=(const float rhs)
{
	x *= rhs;
	y *= rhs;

	return *this;
}

Vector2 Vector2::operator/(const float rhs) const
{
	return { x / rhs, y / rhs };
}

Vector2& Vector2::operator/=(const float rhs)
{
	x /= rhs;
	y /= rhs;

	return *this;
}

float& Vector2::operator[](const int index)
{
	assert(index >= 0 && index < 2 && "Index out of bounds!");

	return (&x)[index];
}

const float& Vector2::operator[](const int index) const
{
	assert(index >= 0 && index < 2 && "Index out of bounds!");

	return (&x)[index];
}

ostream& operator<<(ostream& stream, const Vector2& rhs)
{
	stream << std::format(
		"({:.2f}, {:.2f})", rhs.x, rhs.y
	);

	return stream;
}
