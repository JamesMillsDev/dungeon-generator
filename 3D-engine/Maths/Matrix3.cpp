#include "pch.h"
#include "Matrix3.h"

#include <cassert>

#include "Vector2.h"

Matrix3 Matrix3::MakeTranslate(const Vector2& trans)
{
	return Matrix3
	{
		{ 1.f, 0.f, 0.f },
		{ 0.f, 1.f, 0.f },
		{ trans.x, trans.y, 1.f }
	};
}

Matrix3 Matrix3::MakeTranslate(const float x, const float y)
{
	return MakeTranslate(Vector2{ x, y });
}

Matrix3 Matrix3::MakeRotateX(const float rad)
{
	const float sin = Maths::Sin(rad);
	const float cos = Maths::Cos(rad);

	return Matrix3
	{
		{ 1.f, 0.f, 0.f },
		{ 0.f, cos, sin },
		{ 0.f, -sin, cos }
	};
}

Matrix3 Matrix3::MakeRotateY(const float rad)
{
	const float sin = Maths::Sin(rad);
	const float cos = Maths::Cos(rad);

	return Matrix3
	{
		{ cos, 0.f, -sin },
		{ 0.f, 1.f, 0.f },
		{ sin, 0.f, cos }
	};
}

Matrix3 Matrix3::MakeRotateZ(const float rad)
{
	const float sin = Maths::Sin(rad);
	const float cos = Maths::Cos(rad);

	return Matrix3
	{
		{ cos, sin, 0.f },
		{ -sin, cos, 0.f },
		{ 0.f, 0.f, 1.f }
	};
}

Matrix3 Matrix3::MakeRotate2D(const float rad)
{
	return MakeRotateZ(rad);
}

Matrix3 Matrix3::MakeRotate(const Vector3& euler)
{
	return MakeRotateX(Maths::Radians(euler.x)) *
		MakeRotateY(Maths::Radians(euler.y)) *
		MakeRotateZ(Maths::Radians(euler.z));
}

Matrix3 Matrix3::MakeScale(const Vector2& scale)
{
	return Matrix3
	{
		{ scale.x, 0.f, 0.f },
		{ 0.f, scale.y, 0.f },
		{ 0.f, 0.f, 1.f }
	};
}

Matrix3 Matrix3::MakeScale(const float x, const float y)
{
	return MakeScale(Vector2{ x, y });
}

Matrix3 Matrix3::Make2DTransform(const Vector2& trans, const float rot, const Vector2& scale)
{
	return MakeTranslate(trans) * MakeRotate2D(rot) * MakeScale(scale);
}

Matrix3 Matrix3::Make2DTransform(const float transX, const float transY, const float rot, const float scaleX, const float scaleY)
{
	return MakeTranslate(transX, transY) * MakeRotate2D(rot) * MakeScale(scaleX, scaleY);
}

Matrix3 Matrix3::Identity()
{
	return Matrix3{ 1.f };
}

Matrix3 Matrix3::MakeShear(const Vector2& shear)
{
	return { };
}

Matrix3 Matrix3::MakeShear(float x, float y)
{
	return { };
}

Matrix3::Matrix3()
	: column1{ 0.f, 0.f, 0.f }, column2{ 0.f, 0.f, 0.f }, column3{ 0.f, 0.f, 0.f }
{}

Matrix3::Matrix3(const float scalar)
	: column1{ scalar, 0.f, 0.f }, column2{ 0.f, scalar, 0.f }, column3{ 0.f, 0.f, scalar }
{}

Matrix3::Matrix3(const Vector3& column1, const Vector3& column2, const Vector3& column3)
	: column1{ column1 }, column2{ column2 }, column3{ column3 }
{}

Matrix3::Matrix3(const float _11, const float _12, const float _13,
	const float _21, const float _22, const float _23,
	const float _31, const float _32, const float _33)
	: column1{ _11, _12, _13 }, column2{ _21, _22, _23 }, column3{ _31, _32, _33 }
{}

Matrix3::Matrix3(const mat3& mat)
	: column1{ mat[0] }, column2{ mat[1] }, column3{ mat[2] }
{}

Matrix3::Matrix3(const Matrix3& mat) = default;

Vector3 Matrix3::Translation() const
{
	return column3;
}

float Matrix3::RotationX() const
{
	return Maths::ATan2(column1.y, column1.x);
}

float Matrix3::RotationY() const
{
	return Maths::ATan2(-column2.x, column2.y);
}

float Matrix3::RotationZ() const
{
	return Maths::ATan2(column3.x, column3.z);
}

Vector3 Matrix3::EulerAngles() const
{
	return Vector3
	{
		Maths::Degrees(RotationX()),
		Maths::Degrees(RotationY()),
		Maths::Degrees(RotationZ())
	};
}

Vector3 Matrix3::Scale() const
{
	return Vector3{ column1.Magnitude(), column2.Magnitude(), column3.Magnitude() };
}

Matrix3 Matrix3::Transposed() const
{
	return Matrix3
	{
		{ column1.x, column2.x, column3.x },
		{ column1.y, column2.y, column3.y },
		{ column1.z, column2.z, column3.z }
	};
}

Matrix3 Matrix3::Inverse() const
{
	const Matrix3 adjugate = Adjugate();
	return adjugate / Determinant();
}

Matrix3 Matrix3::Minor() const
{
	return Matrix3
	{
		column2.y * column3.z - column2.z * column3.y,
		column2.x * column3.z - column2.z * column3.x,
		column2.x * column3.y - column2.y * column3.x,
		column1.y * column3.z - column1.z * column3.y,
		column1.z * column3.x - column1.x * column3.z,
		column1.x * column3.y - column1.y * column3.x,
		column1.y * column2.z - column1.z * column2.y,
		column1.x * column2.z - column1.z * column2.x,
		column1.x * column2.y - column1.y * column2.x
	};
}

Matrix3 Matrix3::Cofactor() const
{
	const Matrix3 minor = Minor();
	return Matrix3
	{
		+minor.column1.x, -minor.column2.y, +minor.column3.z,
		-minor.column1.x, +minor.column2.y, -minor.column3.z,
		+minor.column1.x, -minor.column2.y, +minor.column3.z,
	};
}

Matrix3 Matrix3::Adjugate() const
{
	const Matrix3 cofactor = Cofactor();
	return cofactor.Transposed();
}

float Matrix3::Determinant() const
{
	return column1.x * (column2.y * column3.z - column2.z * column3.y) -
		column2.x * (column1.y * column3.z - column1.z * column3.y) -
		column3.x * (column1.y * column2.z - column1.z * column2.y);
}

bool Matrix3::IsOrthogonal(const float e) const
{
	return (*this * Transposed()).IsIdentity(e);
}

bool Matrix3::IsIdentity(const float e) const
{
	return Maths::Approx(column1.x, 1.f, e) && Maths::IsNearZero(column2.x, e) && Maths::IsNearZero(column3.x, e) &&
		Maths::IsNearZero(column1.y, e) && Maths::Approx(column2.y, 1.f, e) && Maths::IsNearZero(column3.y, e) &&
		Maths::IsNearZero(column1.z, e) && Maths::IsNearZero(column2.z, e) && Maths::Approx(column3.z, 1.f, e);
}

Matrix3::operator mat3() const
{
	return mat3
	{
		static_cast<vec3>(column1),
		static_cast<vec3>(column2),
		static_cast<vec3>(column3)
	};
}

Matrix3& Matrix3::operator=(const Matrix3& rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	column1 = rhs.column1;
	column2 = rhs.column2;
	column3 = rhs.column3;

	return *this;
}

Vector3 Matrix3::operator*(const Vector3& rhs) const
{
	return Vector3
	{
		rhs[0] * (*this)[0][0] + rhs[0] * (*this)[1][0] + rhs[0] * (*this)[2][0],
		rhs[1] * (*this)[0][0] + rhs[1] * (*this)[1][0] + rhs[1] * (*this)[2][0],
		rhs[2] * (*this)[0][0] + rhs[2] * (*this)[1][0] + rhs[2] * (*this)[2][0]
	};
}

Matrix3 Matrix3::operator*(const Matrix3& rhs) const
{
	return Matrix3
	{
		rhs[0][0] * (*this)[0][0] + rhs[0][1] * (*this)[1][0] + rhs[0][2] * (*this)[2][0],
		rhs[1][0] * (*this)[0][0] + rhs[1][1] * (*this)[1][0] + rhs[1][2] * (*this)[2][0],
		rhs[2][0] * (*this)[0][0] + rhs[2][1] * (*this)[1][0] + rhs[2][2] * (*this)[2][0],

		rhs[0][0] * (*this)[0][1] + rhs[0][1] * (*this)[1][1] + rhs[0][2] * (*this)[0][1],
		rhs[1][0] * (*this)[0][1] + rhs[1][1] * (*this)[1][1] + rhs[1][2] * (*this)[0][1],
		rhs[2][0] * (*this)[0][1] + rhs[2][1] * (*this)[1][1] + rhs[2][2] * (*this)[0][1],

		rhs[0][0] * (*this)[0][2] + rhs[0][1] * (*this)[1][2] + rhs[0][2] * (*this)[2][2],
		rhs[1][0] * (*this)[0][2] + rhs[1][1] * (*this)[1][2] + rhs[1][2] * (*this)[2][2],
		rhs[2][0] * (*this)[0][2] + rhs[2][1] * (*this)[1][2] + rhs[2][2] * (*this)[2][2],
	};
}

Matrix3& Matrix3::operator*=(const Matrix3& rhs)
{
	*this = *this * rhs;
	return *this;
}

Matrix3 Matrix3::operator/(const float rhs) const
{
	return Matrix3
	{
		column1 / rhs,
		column2 / rhs,
		column3 / rhs
	};
}

bool Matrix3::operator==(const Matrix3& rhs) const
{
	if (this == &rhs)
	{
		return true;
	}

	return column1 == rhs.column1 && column2 == rhs.column3 && column3 == rhs.column3;
}

bool Matrix3::operator!=(const Matrix3& rhs) const
{
	if (this == &rhs)
	{
		return false;
	}

	return column1 != rhs.column1 || column2 != rhs.column3 || column3 != rhs.column3;
}

Vector3& Matrix3::operator[](const int index)
{
	assert(index >= 0 && index <= 3 && "Index out of range!");

	return (&column1)[index];
}

const Vector3& Matrix3::operator[](const int index) const
{
	assert(index >= 0 && index <= 3 && "Index out of range!");

	return (&column1)[index];
}

ostream& operator<<(ostream& stream, const Matrix3& matrix)
{
	stream << std::format(
		"[{:.2f}, {:.2f}, {:.2f}]\n[{:.2f}, {:.2f}, {:.2f}]\n[{:.2f}, {:.2f}, {:.2f}]",
		matrix.column1.x, matrix.column2.x, matrix.column3.x,
		matrix.column1.y, matrix.column2.y, matrix.column3.y,
		matrix.column1.z, matrix.column2.z, matrix.column3.z
	);

	return stream;
}
