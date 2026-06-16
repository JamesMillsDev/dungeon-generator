#pragma once

#include <ostream>

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>

struct Vector2;
struct Vector3;

using std::ostream;

using glm::vec2;
using glm::vec3;
using glm::vec4;

struct Vector4
{
public:
	static const Vector4 ONE;
	static const Vector4 HALF;
	static const Vector4 ZERO;
	static const Vector4 UP;
	static const Vector4 DOWN;
	static const Vector4 LEFT;
	static const Vector4 RIGHT;
	static const Vector4 FORWARD;
	static const Vector4 BACK;

public:
	static Vector4 Lerp(const Vector4& a, const Vector4& b, float t);

	static Vector4 LerpUnclamped(const Vector4& a, const Vector4& b, float t);

	static Vector4 MoveTowards(const Vector4& current, const Vector4& target, float maxDistanceDelta);

	static float Dot(const Vector4& a, const Vector4& b);

	static Vector4 Cross(const Vector4& a, const Vector4& b);

	static Vector4 ClampMagnitude(const Vector4& vec, float maxLength);

public:
	float x;
	float y;
	float z;
	float w;

public:
	Vector4();

	explicit Vector4(float scalar);

	Vector4(float x, float y, float z, float w);

	explicit Vector4(const vec2& vec);

	explicit Vector4(const vec3& vec);

	explicit Vector4(const vec4& vec);

	explicit Vector4(const Vector2& vec);

	explicit Vector4(const Vector3& vec);

	Vector4(const Vector4& rhs);

public:
	void Normalise();

	Vector4 Normalised() const;

	float Magnitude() const;

	float MagnitudeSqr() const;

public:
	explicit operator vec2() const;

	explicit operator vec3() const;

	explicit operator vec4() const;

	explicit operator Vector2() const;

	explicit operator Vector3() const;

	Vector4& operator=(const Vector4& rhs);

	bool operator==(const Vector4& rhs) const;

	bool operator!=(const Vector4& rhs) const;

	Vector4 operator+(const Vector4& rhs) const;

	Vector4& operator+=(const Vector4& rhs);

	Vector4 operator-(const Vector4& rhs) const;

	Vector4& operator-=(const Vector4& rhs);

	Vector4 operator*(const Vector4& rhs) const;

	Vector4& operator*=(const Vector4& rhs);

	Vector4 operator*(float rhs) const;

	Vector4& operator*=(float rhs);

	Vector4 operator/(float rhs) const;

	Vector4& operator/=(float rhs);

	float& operator[](int index);

	const float& operator[](int index) const;

	friend ostream& operator<<(ostream& stream, const Vector4& rhs);

};