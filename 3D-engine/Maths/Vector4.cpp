#include "pch.h"
#include "Vector4.h"

#include <cassert>

#include "Maths/Maths.h"
#include "Maths/Vector2.h"
#include "Maths/Vector3.h"

constexpr float EPSILON_NORMAL_SQRT = 1e-15f;

const Vector4 Vector4::ONE = Vector4{ 1.f };
const Vector4 Vector4::HALF = Vector4{ .5f };
const Vector4 Vector4::ZERO = Vector4{ 0.f };
const Vector4 Vector4::UP = Vector4{ 0.f, 1.f, 0.f, 0.f };
const Vector4 Vector4::DOWN = Vector4{ 0.f, -1.f, 0.f, 0.f };
const Vector4 Vector4::RIGHT = Vector4{ 1.f, 0.f, 0.f, 0.f };
const Vector4 Vector4::LEFT = Vector4{ -1.f, 0.f, 0.f, 0.f };
const Vector4 Vector4::FORWARD = Vector4{ 0.f, 0.f, 1.f, 0.f };
const Vector4 Vector4::BACK = Vector4{ 0.f, 0.f, -1.f, 0.f };

Vector4 Vector4::Lerp(const Vector4& a, const Vector4& b, const float t)
{
	return a + (b - a) * Maths::Clamp01(t);
}

Vector4 Vector4::LerpUnclamped(const Vector4& a, const Vector4& b, const float t)
{
	return a + (b - a) * t;
}

Vector4 Vector4::MoveTowards(const Vector4& current, const Vector4& target, const float maxDistanceDelta)
{
	const float toVectorX = target.x - current.x;
	const float toVectorY = target.y - current.y;
	const float toVectorZ = target.z - current.z;
	const float toVectorW = target.w - current.w;

	const float sqrDist = toVectorX * toVectorX + toVectorY * toVectorY + toVectorZ * toVectorZ + toVectorW * toVectorW;

	if (Maths::IsNearZero(sqrDist) || (maxDistanceDelta >= 0 && sqrDist <= maxDistanceDelta * maxDistanceDelta))
	{
		return target;
	}

	const float dist = Maths::Sqrt(sqrDist);

	return Vector4
	{
		current.x + toVectorX / dist * maxDistanceDelta,
		current.y + toVectorY / dist * maxDistanceDelta,
		current.z + toVectorZ / dist * maxDistanceDelta,
		current.w + toVectorW / dist * maxDistanceDelta
	};
}

float Vector4::Dot(const Vector4& a, const Vector4& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

Vector4 Vector4::Cross(const Vector4& a, const Vector4& b)
{
	return Vector4
	{
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x,
		0.f
	};
}

Vector4 Vector4::ClampMagnitude(const Vector4& vec, const float maxLength)
{
	const float sqrMag = vec.MagnitudeSqr();
	if (sqrMag > maxLength * maxLength)
	{
		const float mag = Maths::Sqrt(sqrMag);

		const float normX = vec.x / mag;
		const float normY = vec.y / mag;
		const float normZ = vec.z / mag;
		const float normW = vec.w / mag;

		return Vector4
		{
			normX * maxLength,
			normY * maxLength,
			normZ * maxLength,
			normW * maxLength
		};
	}

	return vec;
}

Vector4::Vector4()
	: x{ 0.f }, y{ 0.f }, z{ 0.f }, w{ 0.f }
{}

Vector4::Vector4(const float scalar)
	: x{ scalar }, y{ scalar }, z{ scalar }, w{ scalar }
{}

Vector4::Vector4(const float x, const float y, const float z, const float w)
	: x{ x }, y{ y }, z{ z }, w{ w }
{}

Vector4::Vector4(const vec2& vec)
	: x{ vec.x }, y{ vec.y }, z{ 0.f }, w{ 0.f }
{}

Vector4::Vector4(const vec3& vec)
	: x{ vec.x }, y{ vec.y }, z{ vec.z }, w{ 0.f }
{}

Vector4::Vector4(const vec4& vec)
	: x{ vec.x }, y{ vec.y }, z{ vec.z }, w{ vec.w }
{}

Vector4::Vector4(const Vector2& vec)
	: x{ vec.x }, y{ vec.y }, z{ 0.f }, w{ 0.f }
{}

Vector4::Vector4(const Vector3& vec)
	: x{ vec.x }, y{ vec.y }, z{ vec.z }, w{ 0.f }
{}

Vector4::Vector4(const Vector4& rhs) = default;

void Vector4::Normalise()
{
	const float mag = Magnitude();
	if (Maths::IsNearZero(mag))
	{
		x = 0.f;
		y = 0.f;
		z = 0.f;
		w = 0.f;
	}
	else
	{
		x /= mag;
		y /= mag;
		z /= mag;
	}
}

Vector4 Vector4::Normalised() const
{
	const float mag = Magnitude();

	return Maths::IsNearZero(mag) ? Vector4{ 0.f, 0.f, 0.f, 0.f } : Vector4{ x / mag, y / mag, z / mag, w };
}

float Vector4::Magnitude() const
{
	return Maths::Sqrt(MagnitudeSqr());
}

float Vector4::MagnitudeSqr() const
{
	return Dot(*this, *this);
}

Vector4::operator vec2() const
{
	return vec2{ x, y };
}

Vector4::operator vec3() const
{
	return vec3{ x, y, z };
}

Vector4::operator vec4() const
{
	return vec4{ x, y, z, w };
}

Vector4::operator Vector2() const
{
	return Vector2{ x, y };
}

Vector4::operator Vector3() const
{
	return Vector3{ x, y, z };
}

Vector4& Vector4::operator=(const Vector4& rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	w = rhs.w;

	return *this;
}

bool Vector4::operator==(const Vector4& rhs) const
{
	if (this == &rhs)
	{
		return true;
	}

	return Maths::Approx(x, rhs.x) && Maths::Approx(y, rhs.y) &&
		Maths::Approx(z, rhs.z) && Maths::Approx(w, rhs.w);
}

bool Vector4::operator!=(const Vector4& rhs) const
{
	if (this == &rhs)
	{
		return false;
	}

	return !Maths::Approx(x, rhs.x) || !Maths::Approx(y, rhs.y) ||
		!Maths::Approx(z, rhs.z) || !Maths::Approx(w, rhs.w);
}

Vector4 Vector4::operator+(const Vector4& rhs) const
{
	return { x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w };
}

Vector4& Vector4::operator+=(const Vector4& rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	w += rhs.w;

	return *this;
}

Vector4 Vector4::operator-(const Vector4& rhs) const
{
	return { x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w };
}

Vector4& Vector4::operator-=(const Vector4& rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	w -= rhs.w;

	return *this;
}

Vector4 Vector4::operator*(const Vector4& rhs) const
{
	return { x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w };
}

Vector4& Vector4::operator*=(const Vector4& rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	x *= rhs.x;
	y *= rhs.y;
	z *= rhs.z;
	w *= rhs.w;

	return *this;
}

Vector4 Vector4::operator*(const float rhs) const
{
	return { x * rhs, y * rhs, z * rhs, w * rhs };
}

Vector4& Vector4::operator*=(const float rhs)
{
	x *= rhs;
	y *= rhs;
	z *= rhs;
	w *= rhs;

	return *this;
}

Vector4 Vector4::operator/(const float rhs) const
{
	return { x / rhs, y / rhs, z / rhs, w / rhs };
}

Vector4& Vector4::operator/=(const float rhs)
{
	x /= rhs;
	y /= rhs;
	z /= rhs;
	w /= rhs;

	return *this;
}

float& Vector4::operator[](const int index)
{
	assert(index >= 0 && index <= 3 && "Index out of bounds!");

	return (&x)[index];
}

const float& Vector4::operator[](const int index) const
{
	assert(index >= 0 && index <= 3 && "Index out of bounds!");

	return (&x)[index];
}

ostream& operator<<(ostream& stream, const Vector4& rhs)
{
	stream << std::format(
		"({:.2f}, {:.2f}, {:.2f}, {:.2f})", rhs.x, rhs.y, rhs.z, rhs.w
	);

	return stream;
}
