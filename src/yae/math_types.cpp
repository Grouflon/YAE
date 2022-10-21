#include "math_types.h"

#include <glm/ext/scalar_constants.hpp>
#include <glm/gtx/quaternion.hpp>

namespace yae {

const float PI = glm::pi<float>();
const float R2D = (180.f / PI);
const float D2R = (PI / 180.f);

const float SMALL_NUMBER = (1.e-4f);

const Vector2 Vector2::ZERO = Vector2(0.f);
const Vector2 Vector2::ONE = Vector2(1.f);

const Vector3 Vector3::ZERO = Vector3(0.f);
const Vector3 Vector3::ONE = Vector3(1.f);

const Vector3 Vector3::FORWARD = Vector3(0.f, 0.f, 1.f);
const Vector3 Vector3::UP = Vector3(0.f, 1.f, 0.f);
const Vector3 Vector3::RIGHT = Vector3(1.f, 0.f, 0.f);

const Vector4 Vector4::ZERO = Vector4(0.f);
const Vector4 Vector4::ONE = Vector4(1.f);

const Matrix4 Matrix4::ZERO = Matrix4(0.f);
const Matrix4 Matrix4::IDENTITY = Matrix4(
	1.f, 0.f, 0.f, 0.f,
	0.f, 1.f, 0.f ,0.f,
	0.f, 0.f, 1.f ,0.f,
	0.f, 0.f, 0.f ,1.f
);

Vector3 Quaternion::forward() const
{
	return *this * Vector3::FORWARD;
}

Vector3 Quaternion::up() const
{
	return *this * Vector3::UP;
}

Vector3 Quaternion::right() const
{
	return *this * Vector3::RIGHT;
}

Matrix4 Quaternion::toMatrix4() const
{
	return glm::toMat4(*this);
}

const Quaternion Quaternion::IDENTITY = Quaternion(0.f, 0.f, 0.f, 1.f);

String toString(const Vector2& _v)
{
	return string::format("{%.2f, %.2f}", _v.x, _v.y);
}

String toString(const Vector3& _v)
{
	return string::format("{%.2f, %.2f, %.2f}", _v.x, _v.y, _v.z);
}

String toString(const Vector4& _v)
{
	return string::format("{%.2f, %.2f, %.2f, %.2f}", _v.x, _v.y, _v.z, _v.w);
}

} // namespace yae
