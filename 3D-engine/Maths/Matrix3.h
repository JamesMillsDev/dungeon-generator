#pragma once

#include <ostream>

#include <glm/ext/matrix_float3x3.hpp>

#include "Maths/Maths.h"
#include "Maths/Vector3.h"

using std::ostream;

using glm::mat3;

struct Vector2;

struct Matrix3
{
public:
	static Matrix3 MakeTranslate(const Vector2& trans);

	static Matrix3 MakeTranslate(float x, float y);

	static Matrix3 MakeRotateX(float rad);

	static Matrix3 MakeRotateY(float rad);

	static Matrix3 MakeRotateZ(float rad);

	static Matrix3 MakeRotate2D(float rad);

	static Matrix3 MakeRotate(const Vector3& euler);

	static Matrix3 MakeScale(const Vector2& scale);

	static Matrix3 MakeScale(float x, float y);

	static Matrix3 Make2DTransform(const Vector2& trans, float rot, const Vector2& scale);

	static Matrix3 Make2DTransform(float transX, float transY, float rot, float scaleX, float scaleY);

	static Matrix3 Identity();

	static Matrix3 MakeShear(const Vector2& shear);

	static Matrix3 MakeShear(float x, float y);

public:
	Vector3 column1;
	Vector3 column2;
	Vector3 column3;

public:
	Matrix3();

	Matrix3(float scalar);

	Matrix3(const Vector3& column1, const Vector3& column2, const Vector3& column3);

	Matrix3(float _11, float _12, float _13,
		float _21, float _22, float _23,
		float _31, float _32, float _33);

	Matrix3(const mat3& mat);

	Matrix3(const Matrix3& mat);

public:
	[[nodiscard]] Vector3 Translation() const;

	[[nodiscard]] float RotationX() const;

	[[nodiscard]] float RotationY() const;

	[[nodiscard]] float RotationZ() const;

	[[nodiscard]] Vector3 EulerAngles() const;

	[[nodiscard]] Vector3 Scale() const;

	[[nodiscard]] Matrix3 Transposed() const;

	[[nodiscard]] Matrix3 Inverse() const;

	[[nodiscard]] Matrix3 Minor() const;

	[[nodiscard]] Matrix3 Cofactor() const;

	[[nodiscard]] Matrix3 Adjugate() const;

	[[nodiscard]] float Determinant() const;

	[[nodiscard]] bool IsOrthogonal(float e = Maths::EPSILON) const;

	[[nodiscard]] bool IsIdentity(float e = Maths::EPSILON) const;

public:
	explicit operator mat3() const;

	Matrix3& operator=(const Matrix3& rhs);

	Vector3 operator*(const Vector3& rhs) const;

	Matrix3 operator*(const Matrix3& rhs) const;

	Matrix3& operator*=(const Matrix3& rhs);

	Matrix3 operator/(float rhs) const;

	bool operator==(const Matrix3& rhs) const;

	bool operator!=(const Matrix3& rhs) const;

	Vector3& operator[](int index);

	const Vector3& operator[](int index) const;

	friend ostream& operator<<(ostream& stream, const Matrix3& matrix);

};