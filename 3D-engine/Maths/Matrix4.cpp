#include "pch.h"
#include "Matrix4.h"

#include "Vector3.h"
#include "Matrix3.h"

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

Matrix4 Matrix4::MakeTranslate(float x, float y, float z)
{
    return MakeTranslate(Vector3{ x, y, z });
}

Matrix4 Matrix4::MakeRotateX(float rad)
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

Matrix4 Matrix4::MakeRotateY(float rad)
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

Matrix4 Matrix4::MakeRotateZ(float rad)
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

Matrix4 Matrix4::MakeScale(float x, float y, float z)
{
    return MakeScale(Vector3{ x, y, z });
}

Matrix4 Matrix4::MakeTransform(const Vector3& trans, const Vector3& euler, const Vector3& scale)
{
    return MakeTranslate(trans) * MakeRotate(euler) * MakeScale(scale);
}

Matrix4 Matrix4::MakePerspective(float fovY, float aspect, float near, float far)
{
    return Matrix4();
}

Matrix4 Matrix4::MakeOrthographic(float left, float right, float bottom, float top, float near, float far)
{
    return Matrix4();
}

Matrix4 Matrix4::MakeLookAt(const Vector3& eye, const Vector3& target, const Vector3& up)
{
    return Matrix4();
}

Matrix4 Matrix4::Identity()
{
    return Matrix4{ 1.f };
}

Matrix4::Matrix4()
    : column1{ 0.f, 0.f, 0.f, 0.f }, column2{ 0.f, 0.f, 0.f, 0.f }, 
    column3{ 0.f, 0.f, 0.f, 0.f }, column4{ 0.f, 0.f, 0.f, 0.f }
{}

Matrix4::Matrix4(float scalar)
    : column1{ scalar, 0.f, 0.f, 0.f }, column2{ 0.f, scalar, 0.f, 0.f },
    column3{ 0.f, 0.f, scalar, 0.f }, column4{ 0.f, 0.f, 0.f, scalar }
{}

Matrix4::Matrix4(const Vector4& column1, const Vector4& column2, const Vector4& column3, const Vector4& column4)
    : column1{ column1 }, column2{ column2 }, column3{ column3 }, column4{ column4 }
{}

Matrix4::Matrix4(float _11, float _12, float _13, float _14, 
    float _21, float _22, float _23, float _24, 
    float _31, float _32, float _33, float _34, 
    float _41, float _42, float _43, float _44)
    : column1{ _11, _12, _13, _14 }, column2{ _21, _22, _23, _24 }, 
    column3{ _31, _32, _33, _34 }, column4{ _41, _42, _43, _44 }
{}

Matrix4::Matrix4(const mat4& mat)
    : column1{ mat[0] }, column2{ mat[1] }, column3{ mat[2] }, column4{ mat[3] }
{
}

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
    return Vector3{ column1.Magnitude(), column2.Magnitude(), column3.Magnitude() };
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
    return Matrix4
    {

    };
}

Matrix4 Matrix4::Minor() const
{
    return Matrix4();
}

Matrix4 Matrix4::Cofactor() const
{
    return Matrix4();
}

Matrix4 Matrix4::Adjugate() const
{
    return Matrix4
    {

    };
}

float Matrix4::Determinant() const
{
    return 0.0f;
}

bool Matrix4::IsOrthogonal(float e) const
{
    return false;
}

bool Matrix4::IsIdentity(float e) const
{
    return false;
}

Matrix3::operator mat3() const
{}

Matrix3& Matrix3::operator=(const Matrix3 & rhs)
{
    // TODO: insert return statement here
}

void Matrix4::Orthogonalize()
{}

void Matrix4::Orthonormalize()
{}

float Matrix4::Trace() const
{
    return 0.0f;
}

Matrix4::operator mat4() const
{}

Matrix4 Matrix4::operator*(const Matrix4 & rhs) const
{
    return Matrix4();
}

Matrix4& Matrix4::operator*=(const Matrix4 & rhs)
{
    // TODO: insert return statement here
}

bool Matrix4::operator==(const Matrix4 & rhs) const
{
    return false;
}

bool Matrix4::operator!=(const Matrix4 & rhs) const
{
    return false;
}

Matrix4& Matrix4::operator=(const Matrix4 & rhs)
{
    // TODO: insert return statement here
}

ostream& operator<<(ostream& stream, const Matrix4& matrix)
{
    // TODO: insert return statement here
}
