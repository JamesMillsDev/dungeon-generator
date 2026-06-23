#include "pch.h"
#include "Matrix4.h"

#include "Maths/Matrix3.h"
#include "Maths/Vector3.h"

Matrix4 Matrix4::MakeTranslate(const Vector3& trans)
{
	return Matrix4
	{
		{ 1.f, 0.f, 0.f, 0.f },
		{ 0.f, 1.f, 0.f, 0.f },
		{ 0.f, 0.f, 1.f, 0.f },
		{ trans.x, trans.y, trans.z, 1.f }
	};
}

Matrix4 Matrix4::MakeTranslate(const float x, const float y, const float z)
{
	return MakeTranslate(Vector3{ x, y, z });
}

Matrix4 Matrix4::MakeRotateX(const float rad)
{
	const float sin = Maths::Sin(rad);
	const float cos = Maths::Cos(rad);

	return Matrix4
	{
		{ 1.f, 0.f, 0.f, 0.f },
		{ 0.f, cos, sin, 0.f },
		{ 0.f, -sin, cos, 0.f },
		{ 0.f, 0.f, 0.f, 1.f }
	};
}

Matrix4 Matrix4::MakeRotateY(const float rad)
{
	const float sin = Maths::Sin(rad);
	const float cos = Maths::Cos(rad);

	return Matrix4
	{
		{ cos, 0.f, -sin, 0.f },
		{ 0.f, 1.f, 0.f, 0.f },
		{ sin, 0.f, cos, 0.f },
		{ 0.f, 0.f, 0.f, 1.f }
	};
}

Matrix4 Matrix4::MakeRotateZ(const float rad)
{
	const float sin = Maths::Sin(rad);
	const float cos = Maths::Cos(rad);

	return Matrix4
	{
		{ cos, sin, 0.f, 0.f },
		{ -sin, cos, 0.f, 0.f },
		{ 0.f, 0.f, 1.f, 0.f },
		{ 0.f, 0.f, 0.f, 1.f }
	};
}

Matrix4 Matrix4::MakeRotate(const Vector3& euler)
{
	return MakeRotateX(Maths::Radians(euler.x)) *
		MakeRotateY(Maths::Radians(euler.y)) *
		MakeRotateZ(Maths::Radians(euler.z));
}

Matrix4 Matrix4::MakeScale(const Vector3& scale)
{
	return Matrix4
	{
		{ scale.x, 0.f, 0.f, 0.f },
		{ 0.f, scale.y, 0.f, 0.f },
		{ 0.f, 0.f, scale.z, 0.f },
		{ 0.f, 0.f, 0.f, 1.f }
	};
}

Matrix4 Matrix4::MakeScale(const float x, const float y, const float z)
{
	return MakeScale(Vector3{ x, y, z });
}

Matrix4 Matrix4::MakeTransform(const Vector3& trans, const Vector3& euler, const Vector3& scale)
{
	return MakeTranslate(trans) * MakeRotate(euler) * MakeScale(scale);
}

Matrix4 Matrix4::MakePerspective(const float fovY, const float aspect, const float near, const float far)
{
	const float tanFov = Maths::Tan(fovY / 2.f);

	return Matrix4
	{
		1.f / (aspect * tanFov), 0.f, 0.f, 0.f,
		0.f, 1.f / tanFov, 0.f, 0.f,
		0.f, 0.f, -(far + near) / (far - near), -1.f,
		0.f, 0.f, -(2.f * far * near) / (far - near), 0.f
	};
}

Matrix4 Matrix4::MakeOrthographic(const float left, const float right, const float bottom, const float top, const float near, const float far)
{
	return Matrix4
	{
		2.f / (right - left), 0.f, 0.f, -((right + left) / (right - left)),
		0.f, 2.f / (top - bottom), 0.f, -((top + bottom) / (top - bottom)),
		0.f, 0.f, -2.f / (far - near), -((far + near) / (far - near)),
		0.f, 0.f, 0.f, 1.f
	};
}

Matrix4 Matrix4::MakeLookAt(const Vector3& eye, const Vector3& center, const Vector3& up)
{
	const Vector3 f = (center - eye).Normalised();
	const Vector3 s = Vector3::Cross(f, up).Normalised();
	const Vector3 u = Vector3::Cross(s, f);

	return Matrix4
	{
		s.x, u.x, -f.x, 0.f,
		s.y, u.y, -f.y, 0.f,
		s.z, u.z, -f.z, 0.f,
		-Vector3::Dot(s, eye), -Vector3::Dot(u, eye), Vector3::Dot(f, eye), 1.f
	};
}

Matrix4 Matrix4::Identity()
{
	return Matrix4{ 1.f };
}

Matrix4::Matrix4()
	: column1{ 0.f, 0.f, 0.f, 0.f }, column2{ 0.f, 0.f, 0.f, 0.f },
	column3{ 0.f, 0.f, 0.f, 0.f }, column4{ 0.f, 0.f, 0.f, 0.f }
{}

Matrix4::Matrix4(const float scalar)
	: column1{ scalar, 0.f, 0.f, 0.f }, column2{ 0.f, scalar, 0.f, 0.f },
	column3{ 0.f, 0.f, scalar, 0.f }, column4{ 0.f, 0.f, 0.f, scalar }
{}

Matrix4::Matrix4(const Vector4& column1, const Vector4& column2, const Vector4& column3, const Vector4& column4)
	: column1{ column1 }, column2{ column2 }, column3{ column3 }, column4{ column4 }
{}

Matrix4::Matrix4(const float _11, const float _12, const float _13, const float _14,
	const float _21, const float _22, const float _23, const float _24,
	const float _31, const float _32, const float _33, const float _34,
	const float _41, const float _42, const float _43, const float _44)
	: column1{ _11, _12, _13, _14 }, column2{ _21, _22, _23, _24 },
	column3{ _31, _32, _33, _34 }, column4{ _41, _42, _43, _44 }
{}

Matrix4::Matrix4(const mat4& mat)
	: column1{ mat[0] }, column2{ mat[1] }, column3{ mat[2] }, column4{ mat[3] }
{}

Matrix4::Matrix4(const Matrix4& mat) = default;

Vector3 Matrix4::Translation() const
{
	return Vector3{ column4 };
}

float Matrix4::RotationX() const
{
	return Maths::ATan2(column1.y, column1.x);
}

float Matrix4::RotationY() const
{
	return Maths::ATan2(-column2.x, column2.y);
}

float Matrix4::RotationZ() const
{
	return Maths::ATan2(column3.x, column3.z);
}

Vector3 Matrix4::EulerAngles() const
{
	return Vector3{ Maths::Degrees(RotationX()), Maths::Degrees(RotationY()), Maths::Degrees(RotationZ()) };
}

Vector3 Matrix4::Scale() const
{
	return Vector3
	{
		Maths::Degrees(RotationX()),
		Maths::Degrees(RotationY()),
		Maths::Degrees(RotationZ())
	};
}

Matrix4 Matrix4::Transposed() const
{
	return Matrix4
	{
		{ column1.x, column2.x, column3.x, column4.x },
		{ column1.y, column2.y, column3.y, column4.y },
		{ column1.z, column2.z, column3.z, column4.z },
		{ column1.w, column2.w, column3.w, column4.w }
	};
}

Matrix4 Matrix4::Inverse() const
{
	const Matrix4 adjugate = Adjugate();
	return adjugate / Determinant();
}

Matrix4 Matrix4::Minor() const
{
	return Matrix4
	{
		Matrix3{ column2.y, column2.z, column2.w, column3.y, column3.z, column3.w, column4.y, column4.z, column4.w }.Determinant(),
		Matrix3{ column1.y, column1.z, column1.w, column3.y, column3.z, column3.w, column4.y, column4.z, column4.w }.Determinant(),
		Matrix3{ column1.y, column1.z, column1.w, column2.y, column2.z, column2.w, column4.y, column4.z, column4.w }.Determinant(),
		Matrix3{ column1.y, column1.z, column1.w, column2.y, column2.z, column2.w, column3.y, column3.z, column3.w, }.Determinant(),
		Matrix3{ column2.x, column2.z, column2.w, column3.x, column3.z, column3.w, column4.x, column4.z, column4.w }.Determinant(),
		Matrix3{ column1.x, column1.z, column1.w, column3.x, column3.z, column3.w, column4.x, column4.z, column4.w }.Determinant(),
		Matrix3{ column1.x, column1.z, column1.w, column2.x, column2.z, column2.w, column4.x, column4.z, column4.w }.Determinant(),
		Matrix3{ column1.x, column1.z, column1.w, column2.x, column2.z, column2.w, column3.x, column3.z, column3.w, }.Determinant(),
		Matrix3{ column2.x, column2.y, column2.w, column3.x, column3.y, column3.w, column4.x, column4.y, column4.w }.Determinant(),
		Matrix3{ column1.x, column1.y, column1.w, column3.x, column3.z, column3.w, column4.x, column4.y, column4.w }.Determinant(),
		Matrix3{ column1.x, column2.x, column4.x, column1.y, column2.y, column4.y, column1.w, column2.w, column4.w }.Determinant(),
		Matrix3{ column1.x, column2.x, column3.x, column1.y, column2.y, column3.y, column1.w, column2.w, column3.w, }.Determinant(),
		Matrix3{ column2.x, column2.y, column2.z, column3.x, column3.y, column3.z, column4.x, column4.y, column4.z }.Determinant(),
		Matrix3{ column1.x, column1.y, column1.z, column3.x, column3.y, column3.z, column4.x, column4.y, column4.z }.Determinant(),
		Matrix3{ column1.x, column2.x, column4.x, column1.y, column2.y, column4.y, column1.z, column2.z, column4.z }.Determinant(),
		Matrix3{ column1.x, column2.x, column3.x, column1.y, column2.y, column3.y, column1.z, column2.z, column3.z, }.Determinant(),
	};
}

Matrix4 Matrix4::Cofactor() const
{
	const Matrix4 minor = Minor();
	return Matrix4
	{
		+minor.column1.x, -minor.column2.y, +minor.column3.z, -minor.column3.w,
		-minor.column1.x, +minor.column2.y, -minor.column3.z, +minor.column3.w,
		+minor.column1.x, -minor.column2.y, +minor.column3.z, -minor.column3.w,
		-minor.column1.x, +minor.column2.y, -minor.column3.z, +minor.column3.w,
	};
}

Matrix4 Matrix4::Adjugate() const
{
	const Matrix4 cofactor = Cofactor();
	return cofactor.Transposed();
}

float Matrix4::Determinant() const
{
	const Matrix4 cofactor = Cofactor();
	return column1.x * cofactor.column1.x + column1.y * cofactor.column1.y + column1.z * cofactor.column1.z + column1.w * cofactor.column1.w;
}

bool Matrix4::IsOrthogonal(const float e) const
{
	return (*this * Transposed()).IsIdentity(e);
}

bool Matrix4::IsIdentity(const float e) const
{
	return Maths::Approx(column1.x, 1.f, e) && Maths::IsNearZero(column2.x, e) && Maths::IsNearZero(column3.x, e) && Maths::IsNearZero(column4.x, e) &&
		Maths::IsNearZero(column1.y, e) && Maths::Approx(column2.y, 1.f, e) && Maths::IsNearZero(column3.y, e) && Maths::IsNearZero(column4.y, e) &&
		Maths::IsNearZero(column1.z, e) && Maths::IsNearZero(column2.z, e) && Maths::Approx(column3.z, 1.f, e) && Maths::IsNearZero(column4.z, e) &&
		Maths::IsNearZero(column1.w, e) && Maths::IsNearZero(column2.w, e) && Maths::IsNearZero(column3.w, e) && Maths::Approx(column4.w, 1.f, e);
}

void Matrix4::Orthogonalize()
{
	// Gram-Schmidt process - make each column perpendicular to all previous columns
	// by subtracting the projection of the column onto each prior basis vector
	const Vector4 u1 = column1;
	const Vector4 u2 = column2;
	const Vector4 u3 = column3;
	const Vector4 u4 = column4;

	// Remove the component of u2 that points along u1
	const Vector4 prj = u1 * (Vector4::Dot(u2, u1) / Vector4::Dot(u1, u1));
	const Vector4 e2 = u2 - prj;

	// Remove the components of u3 that point along u1 and e2
	Vector4 prj1 = u1 * (Vector4::Dot(u3, u1) / Vector4::Dot(u1, u1));
	Vector4 prj2 = e2 * (Vector4::Dot(u3, e2) / Vector4::Dot(e2, e2));
	const Vector4 e3 = u3 - prj1 - prj2;

	// Remove the components of u4 that point along u1, e2 and e3
	prj1 = u1 * (Vector4::Dot(u4, u1) / Vector4::Dot(u1, u1));
	prj2 = e2 * (Vector4::Dot(u4, e2) / Vector4::Dot(e2, e2));
	const Vector4 prj3 = e3 * (Vector4::Dot(u4, e3) / Vector4::Dot(e3, e3));
	const Vector4 e4 = u4 - prj1 - prj2 - prj3;

	column1 = u1;
	column2 = e2;
	column3 = e3;
	column4 = e4;
}

void Matrix4::Orthonormalize()
{
	for (int i = 0; i < 4; ++i)
	{
		(*this)[i] = (*this)[i].Normalised();
	}
}

float Matrix4::Trace() const
{
	return column1.x + column2.y + column3.z + column4.w;
}

Matrix4::operator mat4() const
{
	return mat4
	{
		static_cast<vec4>(column1),
		static_cast<vec4>(column2),
		static_cast<vec4>(column3),
		static_cast<vec4>(column4),
	};
}

Matrix4& Matrix4::operator=(const Matrix4& rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	column1 = rhs.column1;
	column2 = rhs.column2;
	column3 = rhs.column3;
	column4 = rhs.column4;

	return *this;
}

Vector4 Matrix4::operator*(const Vector4& rhs) const
{
	return Vector4
	{
		column1.x * rhs.x + column2.x * rhs.y + column3.x * rhs.z + column4.x * rhs.w,
		column1.y * rhs.x + column2.y * rhs.y + column3.y * rhs.z + column4.y * rhs.w,
		column1.z * rhs.x + column2.z * rhs.y + column3.z * rhs.z + column4.z * rhs.w,
		column1.w * rhs.x + column2.w * rhs.y + column3.w * rhs.z + column4.w * rhs.w,
	};
}

Matrix4 Matrix4::operator*(const Matrix4& rhs) const
{
	return Matrix4
	{
		column1.x * rhs.column1.x + column1.y * rhs.column2.x + column1.z * rhs.column3.x + column1.w * rhs.column4.x,
		column2.x * rhs.column1.x + column2.y * rhs.column2.x + column2.z * rhs.column3.x + column2.w * rhs.column4.x,
		column3.x * rhs.column1.x + column3.y * rhs.column2.x + column3.z * rhs.column3.x + column3.w * rhs.column4.x,
		column4.x * rhs.column1.x + column4.y * rhs.column2.x + column4.z * rhs.column3.x + column4.w * rhs.column4.x,

		column1.x * rhs.column1.y + column1.y * rhs.column2.y + column1.z * rhs.column3.y + column1.w * rhs.column4.y,
		column2.x * rhs.column1.y + column2.y * rhs.column2.y + column2.z * rhs.column3.y + column2.w * rhs.column4.y,
		column3.x * rhs.column1.y + column3.y * rhs.column2.y + column3.z * rhs.column3.y + column3.w * rhs.column4.y,
		column4.x * rhs.column1.y + column4.y * rhs.column2.y + column4.z * rhs.column3.y + column4.w * rhs.column4.y,

		column1.x * rhs.column1.z + column1.y * rhs.column2.z + column1.z * rhs.column3.z + column1.w * rhs.column4.z,
		column2.x * rhs.column1.z + column2.y * rhs.column2.z + column2.z * rhs.column3.z + column2.w * rhs.column4.z,
		column3.x * rhs.column1.z + column3.y * rhs.column2.z + column3.z * rhs.column3.z + column3.w * rhs.column4.z,
		column4.x * rhs.column1.z + column4.y * rhs.column2.z + column4.z * rhs.column3.z + column4.w * rhs.column4.z,

		column1.x * rhs.column1.w + column1.y * rhs.column2.w + column1.z * rhs.column3.w + column1.w * rhs.column4.w,
		column2.x * rhs.column1.w + column2.y * rhs.column2.w + column2.z * rhs.column3.w + column2.w * rhs.column4.w,
		column3.x * rhs.column1.w + column3.y * rhs.column2.w + column3.z * rhs.column3.w + column3.w * rhs.column4.w,
		column4.x * rhs.column1.w + column4.y * rhs.column2.w + column4.z * rhs.column3.w + column4.w * rhs.column4.w
	};
}

Matrix4& Matrix4::operator*=(const Matrix4& rhs)
{
	*this = *this * rhs;
	return *this;
}

Matrix4 Matrix4::operator/(const float rhs) const
{
	return Matrix4
	{
		column1 / rhs,
		column2 / rhs,
		column3 / rhs,
		column4 / rhs
	};
}

bool Matrix4::operator==(const Matrix4& rhs) const
{
	if (this == &rhs)
	{
		return true;
	}

	return column1 == rhs.column1 && column2 == rhs.column3 && column3 == rhs.column3 && column4 == rhs.column4;
}

bool Matrix4::operator!=(const Matrix4& rhs) const
{
	if (this == &rhs)
	{
		return true;
	}

	return column1 != rhs.column1 || column2 != rhs.column3 || column3 != rhs.column3 || column4 != rhs.column4;
}

Vector4& Matrix4::operator[](const int index)
{
	assert(index >= 0 && index <= 4 && "Index out of range!");

	return (&column1)[index];
}

const Vector4& Matrix4::operator[](const int index) const
{
	assert(index >= 0 && index <= 4 && "Index out of range!");

	return (&column1)[index];
}

ostream& operator<<(ostream& stream, const Matrix4& matrix)
{
	stream << std::format(
		"[{:.2f}, {:.2f}, {:.2f}, {:.2f}]\n[{:.2f}, {:.2f}, {:.2f}, {:.2f}]\n[{:.2f}, {:.2f}, {:.2f}, {:.2f}]\n[{:.2f}, {:.2f}, {:.2f}, {:.2f}]",
		matrix.column1.x, matrix.column2.x, matrix.column3.x, matrix.column4.x,
		matrix.column1.y, matrix.column2.y, matrix.column3.y, matrix.column4.y,
		matrix.column1.z, matrix.column2.z, matrix.column3.z, matrix.column4.z,
		matrix.column1.w, matrix.column2.w, matrix.column3.w, matrix.column4.w
	);

	return stream;
}
