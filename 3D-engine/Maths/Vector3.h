#pragma once

#include <string>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct Vector2;
struct Vector4;

using std::ostream;
using std::string;

using glm::vec2;
using glm::vec3;
using glm::vec4;

struct Vector3
{
public:
	static const Vector3 ONE;
	static const Vector3 HALF;
	static const Vector3 ZERO;
	static const Vector3 UP;
	static const Vector3 DOWN;
	static const Vector3 LEFT;
	static const Vector3 RIGHT;
	static const Vector3 FORWARD;
	static const Vector3 BACK;

public:
	static Vector3 Lerp(const Vector3& a, const Vector3& b, float t);

	static Vector3 LerpUnclamped(const Vector3& a, const Vector3& b, float t);

	static Vector3 MoveTowards(const Vector3& current, const Vector3& target, float maxDistanceDelta);

	static Vector3 Reflect(const Vector3& inDirection, const Vector3& inNormal);

	static float Dot(const Vector3& a, const Vector3& b);

	static Vector3 Cross(const Vector3& a, const Vector3& b);

	static float AngleFromTo(const Vector3& from, const Vector3& to);

	static float SignedAngleFromTo(const Vector3& from, const Vector3& to);

	static float Distance(const Vector3& a, const Vector3& b);

	static Vector3 ClampMagnitude(const Vector3& vec, float maxLength);

public:
	float x;
	float y;
	float z;

public:
	Vector3();

	explicit Vector3(float scalar);

	Vector3(float x, float y, float z);

	explicit Vector3(const vec2& vec);

	explicit Vector3(const vec3& vec);

	explicit Vector3(const vec4& vec);

	explicit Vector3(const Vector2& vec);

	explicit Vector3(const Vector4& vec);

	Vector3(const Vector3& rhs);

public:
	void Normalise();

	Vector3 Normalised() const;

	float Magnitude() const;

	float MagnitudeSqr() const;

public:
	explicit operator vec2() const;

	explicit operator vec3() const;

	explicit operator vec4() const;

	explicit operator Vector2() const;

	explicit operator Vector4() const;

	Vector3& operator=(const Vector3& rhs);

	bool operator==(const Vector3& rhs) const;

	bool operator!=(const Vector3& rhs) const;

	Vector3 operator+(const Vector3& rhs) const;

	Vector3& operator+=(const Vector3& rhs);

	Vector3 operator-(const Vector3& rhs) const;

	Vector3& operator-=(const Vector3& rhs);

	Vector3 operator*(const Vector3& rhs) const;

	Vector3& operator*=(const Vector3& rhs);

	Vector3 operator*(float rhs) const;

	Vector3& operator*=(float rhs);

	Vector3 operator/(float rhs) const;

	Vector3& operator/=(float rhs);

	float& operator[](int index);

	const float& operator[](int index) const;

	friend ostream& operator<<(ostream& stream, const Vector3& rhs);

};