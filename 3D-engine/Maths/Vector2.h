#pragma once

#include <ostream>

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>

struct Vector3;
struct Vector4;

using std::ostream;

using glm::vec2;
using glm::vec3;
using glm::vec4;

struct Vector2
{
public:
	static const Vector2 ONE;
	static const Vector2 HALF;
	static const Vector2 ZERO;
	static const Vector2 UP;
	static const Vector2 DOWN;
	static const Vector2 LEFT;
	static const Vector2 RIGHT;

public:
	static Vector2 Lerp(const Vector2& a, const Vector2& b, float t);

	static Vector2 LerpUnclamped(const Vector2& a, const Vector2& b, float t);

	static Vector2 MoveTowards(const Vector2& current, const Vector2& target, float maxDistanceDelta);

	static Vector2 Reflect(const Vector2& inDirection, const Vector2& inNormal);

	static Vector2 Perpendicular(const Vector2& vec);

	static float Dot(const Vector2& a, const Vector2& b);

	static float AngleFromTo(const Vector2& from, const Vector2& to);

	static float SignedAngleFromTo(const Vector2& from, const Vector2& to);

	static float Distance(const Vector2& a, const Vector2& b);

	static Vector2 ClampMagnitude(const Vector2& vec, float maxLength);

public:
	float x;
	float y;

public:
	Vector2();

	explicit Vector2(float scalar);

	Vector2(float x, float y);

	explicit Vector2(const vec2& vec);

	explicit Vector2(const vec3& vec);

	explicit Vector2(const vec4& vec);

	explicit Vector2(const Vector3& vec);

	explicit Vector2(const Vector4& vec);

	Vector2(const Vector2& rhs);

public:
	void Normalise();

	Vector2 Normalised() const;

	float Magnitude() const;

	float MagnitudeSqr() const;

public:
	explicit operator vec2() const;

	explicit operator vec3() const;

	explicit operator vec4() const;

	explicit operator Vector3() const;

	explicit operator Vector4() const;

	Vector2& operator=(const Vector2& rhs);

	bool operator==(const Vector2& rhs) const;

	bool operator!=(const Vector2& rhs) const;

	Vector2 operator+(const Vector2& rhs) const;

	Vector2& operator+=(const Vector2& rhs);

	Vector2 operator-(const Vector2& rhs) const;

	Vector2& operator-=(const Vector2& rhs);

	Vector2 operator*(const Vector2& rhs) const;

	Vector2& operator*=(const Vector2& rhs);

	Vector2 operator*(float rhs) const;

	Vector2& operator*=(float rhs);

	Vector2 operator/(float rhs) const;

	Vector2& operator/=(float rhs);

	float& operator[](int index);

	const float& operator[](int index) const;

	friend ostream& operator<<(ostream& stream, const Vector2& rhs);

};