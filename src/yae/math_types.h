#pragma once

#include <yae/types.h>

namespace yae {

/*
Super useful documentation on matrices and vectors:
- https://web.archive.org/web/20130529234948/http://seanmiddleditch.com/journal/2012/08/matrices-handedness-pre-and-post-multiplication-row-vs-column-major-and-notations/
- https://seanmiddleditch.github.io/matrices-handedness-pre-and-post-multiplication-row-vs-column-major-and-notations/
*/

/*
Matrix Notation: Column major
Matrix Layout:   Column major
Product order:   Post-multiplication (Means that vectors are column vectors)
*/

struct YAE_API Vector2
{
	float x;
	float y;

	Vector2() {}
	Vector2(float _value) : x(_value), y(_value) {}
	Vector2(float _x, float _y) : x(_x), y(_y) {}

	const float* data() const { return (const float*)this; }
	float* data() { return (float*)this; }

	bool operator==(const Vector2& _rhs) const { return x == _rhs.x && y == _rhs.y; }
	bool operator!=(const Vector2& _rhs) const { return !(*this == _rhs); }

	Vector2 operator+(const Vector2& _rhs) const { return Vector2(x + _rhs.x, y + _rhs.y); }
	Vector2 operator-(const Vector2& _rhs) const { return Vector2(x - _rhs.x, y - _rhs.y); }
	Vector2 operator*(const Vector2& _rhs) const { return Vector2(x * _rhs.x, y * _rhs.y); }
	Vector2 operator/(const Vector2& _rhs) const { return Vector2(x / _rhs.x, y / _rhs.y); }

	Vector2 operator+(const float _rhs) const { return Vector2(x + _rhs, y + _rhs); }
	Vector2 operator-(const float _rhs) const { return Vector2(x - _rhs, y - _rhs); }
	Vector2 operator*(const float _rhs) const { return Vector2(x * _rhs, y * _rhs); }
	Vector2 operator/(const float _rhs) const { return Vector2(x / _rhs, y / _rhs); }

	Vector2& operator+=(const Vector2& _rhs) { x += _rhs.x; y += _rhs.y; return *this; }
	Vector2& operator-=(const Vector2& _rhs) { x -= _rhs.x; y -= _rhs.y; return *this; }

	Vector2 operator-() const { return Vector2(-x, -y); }
};
inline Vector2 operator*(float _a, const Vector2& _b) { return _b * _a; }

struct YAE_API Vector3
{
	float x;
	float y;
	float z;

	Vector3() {}
	Vector3(float _value) : x(_value), y(_value), z(_value) {}
	Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

	const float* data() const { return (const float*)this; }
	float* data() { return (float*)this; }

	Vector2 xy() const { return Vector2(x, y); }
	Vector2 xz() const { return Vector2(x, z); }
	Vector2 yx() const { return Vector2(y, x); }
	Vector2 yz() const { return Vector2(y, z); }
	Vector2 zx() const { return Vector2(z, x); }
	Vector2 zy() const { return Vector2(z, y); }

	bool operator==(const Vector3& _rhs) const { return x == _rhs.x && y == _rhs.y && z == _rhs.z; }
	bool operator!=(const Vector3& _rhs) const { return !(*this == _rhs); }

	Vector3 operator+(const Vector3& _rhs) const { return Vector3(x + _rhs.x, y + _rhs.y, z + _rhs.z); }
	Vector3 operator-(const Vector3& _rhs) const { return Vector3(x - _rhs.x, y - _rhs.y, z - _rhs.z); }
	Vector3 operator*(const Vector3& _rhs) const { return Vector3(x * _rhs.x, y * _rhs.y, z * _rhs.z); }
	Vector3 operator/(const Vector3& _rhs) const { return Vector3(x / _rhs.x, y / _rhs.y, z / _rhs.z); }

	Vector3 operator+(const float _rhs) const { return Vector3(x + _rhs, y + _rhs, z + _rhs); }
	Vector3 operator-(const float _rhs) const { return Vector3(x - _rhs, y - _rhs, z - _rhs); }
	Vector3 operator*(const float _rhs) const { return Vector3(x * _rhs, y * _rhs, z * _rhs); }
	Vector3 operator/(const float _rhs) const { return Vector3(x / _rhs, y / _rhs, z / _rhs); }

	Vector3& operator+=(const Vector3& _rhs) { x += _rhs.x; y += _rhs.y; z += _rhs.z; return *this; }
	Vector3& operator-=(const Vector3& _rhs) { x -= _rhs.x; y -= _rhs.y; z -= _rhs.z; return *this; }

	Vector3 operator-() const { return Vector3(-x, -y, -z); }
};
inline Vector3 operator*(float _a, const Vector3& _b) { return _b * _a; }

struct YAE_API Vector4
{
	float x;
	float y;
	float z;
	float w;

	Vector4() {}
	Vector4(float _value) : x(_value), y(_value), z(_value), w(_value) {}
	Vector4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
	Vector4(const Vector3& _xyz, float _w) : x(_xyz.x), y(_xyz.y), z(_xyz.z), w(_w) {}

	const float* data() const { return (const float*)this; }
	float* data() { return (float*)this; }

	Vector3 xyz() const { return Vector3(x, y, z); }

	bool operator==(const Vector4& _rhs) const { return x == _rhs.x && y == _rhs.y && z == _rhs.z && w == _rhs.w; }
	bool operator!=(const Vector4& _rhs) const { return !(*this == _rhs); }

	Vector4 operator+(const Vector4& _rhs) const { return Vector4(x + _rhs.x, y + _rhs.y, z + _rhs.z, w + _rhs.w); }
	Vector4 operator-(const Vector4& _rhs) const { return Vector4(x - _rhs.x, y - _rhs.y, z - _rhs.z, w - _rhs.w); }
	Vector4 operator*(const Vector4& _rhs) const { return Vector4(x * _rhs.x, y * _rhs.y, z * _rhs.z, w * _rhs.w); }
	Vector4 operator/(const Vector4& _rhs) const { return Vector4(x / _rhs.x, y / _rhs.y, z / _rhs.z, w / _rhs.w); }

	Vector4 operator+(const float _rhs) const { return Vector4(x + _rhs, y + _rhs, z + _rhs, w + _rhs); }
	Vector4 operator-(const float _rhs) const { return Vector4(x - _rhs, y - _rhs, z - _rhs, w - _rhs); }
	Vector4 operator*(const float _rhs) const { return Vector4(x * _rhs, y * _rhs, z * _rhs, w * _rhs); }
	Vector4 operator/(const float _rhs) const { return Vector4(x / _rhs, y / _rhs, z / _rhs, w / _rhs); }

	Vector4& operator+=(const Vector4& _rhs) { x += _rhs.x; y += _rhs.y; z += _rhs.z; w += _rhs.w; return *this; }
	Vector4& operator-=(const Vector4& _rhs) { x -= _rhs.x; y -= _rhs.y; z -= _rhs.z; w -= _rhs.w; return *this; }

	Vector4 operator-() const { return Vector4(-x, -y, -z, -w); }
};
inline Vector4 operator*(float _a, const Vector4& _b) { return _b * _a; }

struct YAE_API Quaternion
{
	float x;
	float y;
	float z;
	float w;

	Quaternion() {}
	Quaternion(float _value) : x(_value), y(_value), z(_value), w(_value) {}
	Quaternion(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}

	const float* data() const { return (const float*)this; }
	float* data() { return (float*)this; }

	bool operator==(const Quaternion& _rhs) const { return x == _rhs.x && y == _rhs.y && z == _rhs.z && w == _rhs.w; }
	bool operator!=(const Quaternion& _rhs) const { return !(*this == _rhs); }

	Quaternion operator*(float _s) const { return Quaternion(x*_s, y*_s, z*_s, w*_s); }
	Quaternion operator/(float _s) const { return Quaternion(x/_s, y/_s, z/_s, w/_s); }

	Quaternion operator*(const Quaternion& _rhs) const {
		// from glm/detail/type_quat.inl:285
		Quaternion const p(*this);
		Quaternion const q(_rhs);

		return Quaternion(
			p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y,
			p.w * q.y + p.y * q.w + p.z * q.x - p.x * q.z,
			p.w * q.z + p.z * q.w + p.x * q.y - p.y * q.x,
			p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z
		);
	}
};

struct YAE_API Matrix3
{
	float m[3][3];

	Matrix3() {}
	Matrix3(float _value)
	{
		m[0][0] = _value; m[0][1] = _value; m[0][2] = _value;
		m[1][0] = _value; m[1][1] = _value; m[1][2] = _value;
		m[2][0] = _value; m[2][1] = _value; m[2][2] = _value;
	}
	Matrix3(float _m00, float _m01, float _m02,
			float _m10, float _m11, float _m12,
			float _m20, float _m21, float _m22)
	{
		m[0][0] = _m00; m[0][1] = _m01; m[0][2] = _m02;
		m[1][0] = _m10; m[1][1] = _m11; m[1][2] = _m12;
		m[2][0] = _m20; m[2][1] = _m21; m[2][2] = _m22;
	}

	const float* data() const { return (const float*)this; }
	float* data() { return (float*)this; }

	float(&operator[](size_t _i))[3] { YAE_ASSERT(_i >= 0 && _i < 3); return m[_i]; }
	float const(&operator[](size_t _i) const)[3] { YAE_ASSERT(_i >= 0 && _i < 3); return m[_i]; }
};

struct YAE_API Matrix4
{
	float m[4][4];

	Matrix4() {}
	Matrix4(float _value)
	{
		m[0][0] = _value; m[0][1] = _value; m[0][2] = _value; m[0][3] = _value;
		m[1][0] = _value; m[1][1] = _value; m[1][2] = _value; m[1][3] = _value;
		m[2][0] = _value; m[2][1] = _value; m[2][2] = _value; m[2][3] = _value;
		m[3][0] = _value; m[3][1] = _value; m[3][2] = _value; m[3][3] = _value;
	}
	Matrix4(float _m00, float _m01, float _m02, float _m03,
			float _m10, float _m11, float _m12, float _m13,
			float _m20, float _m21, float _m22, float _m23,
			float _m30, float _m31, float _m32, float _m33)
	{
		m[0][0] = _m00; m[0][1] = _m01; m[0][2] = _m02; m[0][3] = _m03;
		m[1][0] = _m10; m[1][1] = _m11; m[1][2] = _m12; m[1][3] = _m13;
		m[2][0] = _m20; m[2][1] = _m21; m[2][2] = _m22; m[2][3] = _m23;
		m[3][0] = _m30; m[3][1] = _m31; m[3][2] = _m32; m[3][3] = _m33;
	}

	const float* data() const { return (const float*)this; }
	float* data() { return (float*)this; }

	float(&operator[](size_t _i))[4] { YAE_ASSERT(_i >= 0 && _i < 4); return m[_i]; }
	float const(&operator[](size_t _i) const)[4] { YAE_ASSERT(_i >= 0 && _i < 4); return m[_i]; }

	Matrix4 operator*(const Matrix4& _rhs) const
	{
		Matrix4 output;
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				output.m[i][j] = 0;
				for (int k = 0; k < 4; ++k)
				{
					output.m[i][j] += m[k][j] * _rhs.m[i][k];
				}
			}
		}
		return output;
	}

	Vector4 operator*(const Vector4& _rhs) const
	{
		Vector4 output;
		for (int i = 0; i < 4; ++i)
		{
			output.data()[i] = 0.f;
			for (int j = 0; j < 4; ++j)
			{
				output.data()[i] += m[j][i] * _rhs.data()[i];
			}
		}
		return output;
	}

	Vector3 operator*(const Vector3& _rhs) const
	{
		Vector4 v = *this * Vector4(_rhs.x, _rhs.y, _rhs.z, 1.f);
		return v.xyz();
	}

	Vector2 operator*(const Vector2& _rhs) const
	{
		Vector4 v = *this * Vector4(_rhs.x, _rhs.y, 0.f, 1.f);
		return Vector2(v.x, v.y);
	}
};

// CONSTANTS
YAE_API extern const float PI;
YAE_API extern const float D2R;
YAE_API extern const float R2D;

YAE_API extern const float SMALL_NUMBER;
YAE_API extern const float VERY_SMALL_NUMBER;

namespace vector2 {
YAE_API extern const Vector2 ZERO;
YAE_API extern const Vector2 ONE;
YAE_API extern const Vector2 FORWARD;
YAE_API extern const Vector2 UP;
} // namespace vector2

namespace vector3 {
YAE_API extern const Vector3 ZERO;
YAE_API extern const Vector3 ONE;
YAE_API extern const Vector3 FORWARD;
YAE_API extern const Vector3 UP;
YAE_API extern const Vector3 RIGHT;
} // namespace vector3

namespace vector4 {
YAE_API extern const Vector4 ZERO;
YAE_API extern const Vector4 ONE;
} // namespace vector4

namespace quaternion {
YAE_API extern const Quaternion IDENTITY;
} // namespace quaternion

namespace matrix3 {
YAE_API extern const Matrix3 ZERO;
YAE_API extern const Matrix3 IDENTITY;
} // namespace matrix3

namespace matrix4 {
YAE_API extern const Matrix4 ZERO;
YAE_API extern const Matrix4 IDENTITY;
} // namespace matrix4

//YAE_API String toString(const Vector2& _v);
//YAE_API String toString(const Vector3& _v);
//YAE_API String toString(const Vector4& _v);

} // namespace yae
