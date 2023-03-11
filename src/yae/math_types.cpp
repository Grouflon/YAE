#include "math_types.h"

#include <cfloat>

namespace yae {

// CONSTANTS
	
const float PI = 3.14159265358979323846;
const float R2D = (180.f / PI);
const float D2R = (PI / 180.f);

const float SMALL_NUMBER = (1.e-4f);
const float VERY_SMALL_NUMBER = FLT_EPSILON;

namespace vector2 {
const Vector2 ZERO = Vector2(0.f);
const Vector2 ONE = Vector2(1.f);
const Vector2 FORWARD = Vector2(1.f, 0.f);
const Vector2 UP = Vector2(0.f, 1.f);
} // namespace vector2

namespace vector3 {
const Vector3 ZERO = Vector3(0.f);
const Vector3 ONE = Vector3(1.f);
const Vector3 FORWARD = Vector3(1.f, 0.f, 0.f);
const Vector3 UP = Vector3(0.f, 1.f, 0.f);
const Vector3 RIGHT = Vector3(0.f, 0.f, 1.f);
} // namespace vector3

namespace vector4 {
const Vector4 ZERO = Vector4(0.f);
const Vector4 ONE = Vector4(1.f);
} // namespace vector4

namespace quaternion {
const Quaternion IDENTITY = Quaternion(0.f, 0.f, 0.f, 1.f);
} // namespace quaternion

namespace matrix3 {
const Matrix3 ZERO = Matrix3(0.f);
const Matrix3 IDENTITY = Matrix3(
	1.f, 0.f, 0.f,
	0.f, 1.f, 0.f,
	0.f, 0.f, 1.f
);
} // namespace matrix3

namespace matrix4 {
const Matrix4 ZERO = Matrix4(0.f);
const Matrix4 IDENTITY = Matrix4(
	1.f, 0.f, 0.f, 0.f,
	0.f, 1.f, 0.f ,0.f,
	0.f, 0.f, 1.f ,0.f,
	0.f, 0.f, 0.f ,1.f
);
} // namespace matrix4

/*
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
*/

} // namespace yae
