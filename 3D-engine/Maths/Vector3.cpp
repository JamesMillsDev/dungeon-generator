#include "pch.h"
#include "Vector3.h"

#include "Maths.h"
#include "Vector2.h"
#include "Vector4.h"

constexpr float EPSILON_NORMAL_SQRT = 1e-15f;

const Vector3 Vector3::ONE = Vector3{ 1.f };
const Vector3 Vector3::HALF = Vector3{ .5f };
const Vector3 Vector3::ZERO = Vector3{ 0.f };
const Vector3 Vector3::UP = Vector3{ 0.f, 1.f, 0.f };
const Vector3 Vector3::DOWN = Vector3{ 0.f, -1.f, 0.f };
const Vector3 Vector3::RIGHT = Vector3{ 1.f, 0.f, 0.f };
const Vector3 Vector3::LEFT = Vector3{ -1.f, 0.f, 0.f };
const Vector3 Vector3::FORWARD = Vector3{ 0.f, 0.f, 1.f };
const Vector3 Vector3::BACK = Vector3{ 0.f, 0.f, -1.f };

Vector3 Vector3::Lerp(const Vector3& a, const Vector3& b, const float t)
{
	return a + (b - a) * Maths::Clamp01(t);
}

Vector3 Vector3::LerpUnclamped(const Vector3& a, const Vector3& b, const float t)
{
	return a + (b - a) * t;
}

Vector3 Vector3::MoveTowards(const Vector3& current, const Vector3& target, const float maxDistanceDelta)
{
	const float toVectorX = target.x - current.x;
	const float toVectorY = target.y - current.y;
	const float toVectorZ = target.z - current.z;

	const float sqrDist = toVectorX * toVectorX + toVectorY * toVectorY + toVectorZ * toVectorZ;

	if (Maths::IsNearZero(sqrDist) || (maxDistanceDelta >= 0 && sqrDist <= maxDistanceDelta * maxDistanceDelta))
	{
		return target;
	}

	const float dist = Maths::Sqrt(sqrDist);

	return Vector3
	{
		current.x + toVectorX / dist * maxDistanceDelta,
		current.y + toVectorY / dist * maxDistanceDelta,
		current.z + toVectorZ / dist * maxDistanceDelta
	};
}

Vector3 Vector3::Reflect(const Vector3& inDirection, const Vector3& inNormal)
{
	const float factor = -2.f * Dot(inNormal, inDirection);
	return Vector3
	{
		factor * inNormal.x + inDirection.x,
		factor * inNormal.y + inDirection.y,
		factor * inNormal.z + inDirection.z
	};
}

float Vector3::Dot(const Vector3& a, const Vector3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector3 Vector3::Cross(const Vector3& a, const Vector3& b)
{
	return Vector3
	{
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

float Vector3::AngleFromTo(const Vector3& from, const Vector3& to)
{
	const float denominator = Maths::Sqrt(from.MagnitudeSqr() * to.MagnitudeSqr());
	if (denominator < EPSILON_NORMAL_SQRT)
	{
		return 0.f;
	}

	const float dot = Maths::Clamp(Dot(from, to) / denominator, -1.f, 1.f);
	return Maths::Degrees(Maths::ACos(dot));
}

float Vector3::SignedAngleFromTo(const Vector3& from, const Vector3& to)
{
	const float unsignedAngle = AngleFromTo(from, to);
	const float sign = Maths::Sign(from.x * to.y - from.y * to.x);

	return unsignedAngle * sign;
}

float Vector3::Distance(const Vector3& a, const Vector3& b)
{
	const float diffX = a.x - b.x;
	const float diffY = a.y - b.y;
	const float diffZ = a.z - b.z;

	return Maths::Sqrt(diffX * diffX + diffY * diffY + diffZ * diffZ);
}

Vector3 Vector3::ClampMagnitude(const Vector3& vec, const float maxLength)
{
	const float sqrMag = vec.MagnitudeSqr();
	if (sqrMag > maxLength * maxLength)
	{
		const float mag = Maths::Sqrt(sqrMag);

		const float normX = vec.x / mag;
		const float normY = vec.y / mag;
		const float normZ = vec.z / mag;

		return Vector3
		{
			normX * maxLength,
			normY * maxLength,
			normZ * maxLength
		};
	}

	return vec;
}

Vector3::Vector3()
	: x{ 0.f }, y{ 0.f }, z{ 0.f }
{ }

Vector3::Vector3(const float scalar)
	: x{ scalar }, y{ scalar }, z{ scalar }
{ }

Vector3::Vector3(const float x, const float y, const float z)
	: x{ x }, y{ y }, z{ z }
{ }

Vector3::Vector3(const vec2& vec)
	: x{ vec.x }, y{ vec.y }, z{ 0.f }
{ }

Vector3::Vector3(const vec3& vec)
	: x{ vec.x }, y{ vec.y }, z{ vec.z }
{ }

Vector3::Vector3(const vec4& vec)
	: x{ vec.x }, y{ vec.y }, z{ vec.z }
{ }

Vector3::Vector3(const Vector2& vec)
	: x{ vec.x }, y{ vec.y }, z{ 0.f }
{ }

Vector3::Vector3(const Vector4& vec)
	: x{ vec.x }, y{ vec.y }, z{ vec.z }
{ }

Vector3::Vector3(const Vector3& rhs) = default;

void Vector3::Normalise()
{
	const float mag = Magnitude();
	if (Maths::IsNearZero(mag))
	{
		x = 0.f;
		y = 0.f;
		z = 0.f;
	}
	else
	{
		x /= mag;
		y /= mag;
		z /= mag;
	}
}

Vector3 Vector3::Normalised() const
{
	const float mag = Magnitude();

	return Maths::IsNearZero(mag) ? Vector3{ 0.f, 0.f, 0.f } : Vector3{ x / mag, y / mag, z / mag };
}

float Vector3::Magnitude() const
{
	return Maths::Sqrt(MagnitudeSqr());
}

float Vector3::MagnitudeSqr() const
{
	return Dot(*this, *this);
}

Vector3::operator vec2() const
{
	return vec2{ x, y };
}

Vector3::operator vec3() const
{
	return vec3{ x, y, z };
}

Vector3::operator vec4() const
{
	return vec4{ x, y, z, 0.f };
}

Vector3::operator Vector2() const
{
	return Vector2{ x, y };
}

Vector3::operator Vector4() const
{
	return Vector4{ x, y, z, 0.f };
}

Vector3& Vector3::operator=(const Vector3& rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	x = rhs.x;
	y = rhs.y;
	z = rhs.z;

	return *this;
}

bool Vector3::operator==(const Vector3& rhs) const
{
	if (this == &rhs)
	{
		return true;
	}

	return Maths::Approx(x, rhs.x) && Maths::Approx(y, rhs.y) && Maths::Approx(z, rhs.z);
}

bool Vector3::operator!=(const Vector3& rhs) const
{
	if (this == &rhs)
	{
		return false;
	}

	return !Maths::Approx(x, rhs.x) || !Maths::Approx(y, rhs.y) || !Maths::Approx(z, rhs.z);
}

Vector3 Vector3::operator+(const Vector3& rhs) const
{
	return { x + rhs.x, y + rhs.y, z + rhs.z };
}

Vector3& Vector3::operator+=(const Vector3& rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	x += rhs.x;
	y += rhs.y;
	z += rhs.z;

	return *this;
}

Vector3 Vector3::operator-(const Vector3& rhs) const
{
	return { x - rhs.x, y - rhs.y, z - rhs.z };
}

Vector3& Vector3::operator-=(const Vector3& rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;

	return *this;
}

Vector3 Vector3::operator*(const Vector3& rhs) const
{
	return { x * rhs.x, y * rhs.y, z * rhs.z };
}

Vector3& Vector3::operator*=(const Vector3& rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	x *= rhs.x;
	y *= rhs.y;
	z *= rhs.z;

	return *this;
}

Vector3 Vector3::operator*(const float rhs) const
{
	return { x * rhs, y * rhs, z * rhs };
}

Vector3& Vector3::operator*=(const float rhs)
{
	x *= rhs;
	y *= rhs;
	z *= rhs;

	return *this;
}

Vector3 Vector3::operator/(const float rhs) const
{
	return { x / rhs, y / rhs, z / rhs };
}

Vector3& Vector3::operator/=(const float rhs)
{
	x /= rhs;
	y /= rhs;
	z /= rhs;

	return *this;
}

float& Vector3::operator[](const int index)
{
	assert(index >= 0 && index < 3 && "Index out of bounds!");

	return (&x)[index];
}

const float& Vector3::operator[](const int index) const
{
	assert(index >= 0 && index < 3 && "Index out of bounds!");

	return (&x)[index];
}

ostream& operator<<(ostream& stream, const Vector3& rhs)
{
	stream << std::format(
		"({:.2f}, {:.2f}, {:.2f})", rhs.x, rhs.y, rhs.z
	);

	return stream;
}
