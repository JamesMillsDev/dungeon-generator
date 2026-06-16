#pragma once

#include <ostream>

#include <glm/ext/matrix_float4x4.hpp>

#include "Maths.h"
#include "Vector4.h"

using std::ostream;

using glm::mat4;

struct Matrix3;
struct Vector3;

struct Matrix4
{
public:
	static Matrix4 MakeTranslate(const Vector3& trans);

	static Matrix4 MakeTranslate(float x, float y, float z);

	static Matrix4 MakeRotateX(float rad);

	static Matrix4 MakeRotateY(float rad);

	static Matrix4 MakeRotateZ(float rad);

	static Matrix4 MakeRotate(const Vector3& euler);

	static Matrix4 MakeScale(const Vector3& scale);

	static Matrix4 MakeScale(float x, float y, float z);

	static Matrix4 MakeTransform(const Vector3& trans, const Vector3& euler, const Vector3& scale);

	static Matrix4 MakePerspective(float fovY, float aspect, float near, float far);

	static Matrix4 MakeOrthographic(float left, float right, float bottom, float top, float near, float far);

	static Matrix4 MakeLookAt(const Vector3& eye, const Vector3& target, const Vector3& up);

	static Matrix4 Identity();

public:
	Vector4 column1;
	Vector4 column2;
	Vector4 column3;
	Vector4 column4;

public:
	Matrix4();

	Matrix4(float scalar);

	Matrix4(const Vector4& column1, const Vector4& column2, const Vector4& column3, const Vector4& column4);

	Matrix4(float _11, float _12, float _13, float _14,
		float _21, float _22, float _23, float _24,
		float _31, float _32, float _33, float _34,
		float _41, float _42, float _43, float _44);

	Matrix4(const mat4& mat);

	Matrix4(const Matrix4& mat);

public:
	[[nodiscard]] Vector3 Translation() const;

	[[nodiscard]] float RotationX() const;

	[[nodiscard]] float RotationY() const;

	[[nodiscard]] float RotationZ() const;

	[[nodiscard]] Vector3 EulerAngles() const;

	[[nodiscard]] Vector3 Scale() const;

	[[nodiscard]] Matrix4 Transposed() const;

	[[nodiscard]] Matrix4 Inverse() const;

	[[nodiscard]] Matrix4 Minor() const;

	[[nodiscard]] Matrix4 Cofactor() const;

	[[nodiscard]] Matrix4 Adjugate() const;

	[[nodiscard]] float Determinant() const;

	[[nodiscard]] bool IsOrthogonal(float e = Maths::EPSILON) const;

	[[nodiscard]] bool IsIdentity(float e = Maths::EPSILON) const;

	void Orthogonalize();

	void Orthonormalize();

	[[nodiscard]] float Trace() const;

public:
	explicit operator mat4() const;

	Matrix4& operator=(const Matrix4& rhs);

	Vector4 operator*(const Vector4& rhs) const;

	Matrix4 operator*(const Matrix4& rhs) const;

	Matrix4& operator*=(const Matrix4& rhs);

	bool operator==(const Matrix4& rhs) const;

	bool operator!=(const Matrix4& rhs) const;

	Vector4& operator[](int index);

	const Vector4& operator[](int index) const;

	friend ostream& operator<<(ostream& stream, const Matrix4& matrix);

};