#include "math.h"

#include <cmath>
#include <algorithm>

namespace yae {

i8     abs(i8 _value)     { return std::abs(_value); }
i16    abs(i16 _value)    { return std::abs(_value); }
i32    abs(i32 _value)    { return std::abs(_value); }
i64    abs(i64 _value)    { return std::abs(_value); }
float  abs(float _value)  { return std::abs(_value); }
double abs(double _value) { return std::abs(_value); }

u8     min(u8 _a, u8 _b)         { return std::min(_a, _b); }
u16    min(u16 _a, u16 _b)       { return std::min(_a, _b); }
u32    min(u32 _a, u32 _b)       { return std::min(_a, _b); }
u64    min(u64 _a, u64 _b)       { return std::min(_a, _b); }
i8     min(i8 _a, i8 _b)         { return std::min(_a, _b); }
i16    min(i16 _a, i16 _b)       { return std::min(_a, _b); }
i32    min(i32 _a, i32 _b)       { return std::min(_a, _b); }
i64    min(i64 _a, i64 _b)       { return std::min(_a, _b); }
float  min(float _a, float _b)   { return std::min(_a, _b); }
double min(double _a, double _b) { return std::min(_a, _b); }

u8     max(u8 _a, u8 _b)         { return std::max(_a, _b); }
u16    max(u16 _a, u16 _b)       { return std::max(_a, _b); }
u32    max(u32 _a, u32 _b)       { return std::max(_a, _b); }
u64    max(u64 _a, u64 _b)       { return std::max(_a, _b); }
i8     max(i8 _a, i8 _b)         { return std::max(_a, _b); }
i16    max(i16 _a, i16 _b)       { return std::max(_a, _b); }
i32    max(i32 _a, i32 _b)       { return std::max(_a, _b); }
i64    max(i64 _a, i64 _b)       { return std::max(_a, _b); }
float  max(float _a, float _b)   { return std::max(_a, _b); }
double max(double _a, double _b) { return std::max(_a, _b); }

float clamp(float _value, float _min, float _max)
{
	YAE_ASSERT(_min <= _max);
	return max(min(_value, _max), _min);
}

float mod(float _a, float _b)
{
	return std::fmod(_a, _b);
}

float positiveMod(float _a, float _b)
{
	return mod(mod(_a, _b) + _b, _b);
}

bool isZero(float _value, float _threshold)
{
	return abs(_value) <= _threshold;
}

bool isZero(double _value, double _threshold)
{
	return abs(_value) <= _threshold;
}

bool isZero(const Vector2& _v, float _threshold)
{
	return isZero(_v.x) && isZero(_v.y);
}

bool isZero(const Vector3& _v, float _threshold)
{
	return isZero(_v.x) && isZero(_v.y) && isZero(_v.z);
}

bool isZero(const Vector4& _v, float _threshold)
{
	return isZero(_v.x) && isZero(_v.y) && isZero(_v.z) && isZero(_v.w);
}

Vector2 normalize(const Vector2& _v)
{
	return glm::normalize(_v);
}

Vector3 normalize(const Vector3& _v)
{
	return glm::normalize(_v);
}

Vector4 normalize(const Vector4& _v)
{
	return glm::normalize(_v);
}

Vector2 safeNormalize(const Vector2& _v, float _threshold)
{
	if (isZero(_v, _threshold))
		return Vector2::ZERO;
	return normalize(_v);
}

Vector3 safeNormalize(const Vector3& _v, float _threshold)
{
	if (isZero(_v, _threshold))
		return Vector3::ZERO;
	return normalize(_v);
}

Vector4 safeNormalize(const Vector4& _v, float _threshold)
{
	if (isZero(_v, _threshold))
		return Vector4::ZERO;
	return normalize(_v);
}

Matrix4 inverse(const Matrix4& _m)
{
	return glm::inverse(_m);
}

Matrix4 toMatrix4(const Quaternion& _q)
{
	return glm::mat4_cast(_q);
}

Matrix4 makeTransformMatrix(const Vector3& _position, const Quaternion& _rotation, const Vector3& _scale)
{
	Matrix4 t = Matrix4(1.f, 0.f, 0.f, 0.f,  0.f, 1.f, 0.f, 0.f,  0.f, 0.f, 1.f, 0.f,  _position.x, _position.y, _position.z, 1.f);
	Matrix4 r = toMatrix4(_rotation);
	Matrix4 s = Matrix4(_scale.x, 0.f, 0.f, 0.f,  0.f, _scale.y, 0.f, 0.f,  0.f, 0.f, _scale.z, 0.f,  0.f, 0.f, 0.f, 1.f);
	Matrix4 m = t * r * s;

	return m;
}

} // namespace yae
