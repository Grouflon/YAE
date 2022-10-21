#pragma once

#include <yae/types.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

namespace yae {

YAE_API extern const float PI;
YAE_API extern const float D2R;
YAE_API extern const float R2D;

YAE_API extern const float SMALL_NUMBER;

struct YAE_API Vector2 : public glm::vec2
{
	Vector2() {}
	Vector2(float _value) { x = _value; y = _value; }
	Vector2(float _x, float _y) { x = _x; y = _y; }
	Vector2(const glm::vec2& _v) : Vector2(_v.x, _v.y) {}

	static const Vector2 ZERO;
	static const Vector2 ONE;
};


struct YAE_API Vector3 : public glm::vec3
{
	Vector3() {}
	Vector3(float _value) { x = _value; y = _value; z = _value; }
	Vector3(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }
	Vector3(const glm::vec3& _v) : Vector3(_v.x, _v.y, _v.z) {}

	Vector2 xy() const { return Vector2(x,y); }
	Vector2 xz() const { return Vector2(x,z); }

	Vector3& operator+=(const Vector3& _rhs) { x += _rhs.x; y += _rhs.y; z += _rhs.z; return *this; }
	Vector3& operator-=(const Vector3& _rhs) { x -= _rhs.x; y -= _rhs.y; z -= _rhs.z; return *this; }

	static const Vector3 ZERO;
	static const Vector3 ONE;

	static const Vector3 FORWARD;
	static const Vector3 UP;
	static const Vector3 RIGHT;
};


struct YAE_API Vector4 : public glm::vec4
{
	Vector4() {}
	Vector4(float _value) { x = _value; y = _value; z = _value; w = _value; }
	Vector4(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }
	Vector4(const Vector3& _v, float _w) : Vector4(_v.x, _v.y, _v.z, _w) {}
	Vector4(const glm::vec4& _v) : Vector4(_v.x, _v.y, _v.z, _v.w) {}

	Vector3 xyz() const { return Vector3(x,y,z); }

	static const Vector4 ZERO;
	static const Vector4 ONE;
};

struct YAE_API Matrix4 : public glm::mat4x4
{
	Matrix4() {}
	Matrix4(float _value)
	{
		for(int y = 0; y < 4; ++y)
		for(int x = 0; x < 4; ++x)
		{
			(*this)[x][y] = _value;
		}
	}
	Matrix4(
		float _x0, float _y0, float _z0, float _w0,
		float _x1, float _y1, float _z1, float _w1,
		float _x2, float _y2, float _z2, float _w2,
		float _x3, float _y3, float _z3, float _w3
	) : glm::mat4x4(
		_x0, _y0, _z0, _w0,
		_x1, _y1, _z1, _w1,
		_x2, _y2, _z2, _w2,
		_x3, _y3, _z3, _w3) {}
	Matrix4(const glm::mat4x4& _m) : Matrix4(
		_m[0][0], _m[0][1], _m[0][2], _m[0][3],
		_m[1][0], _m[1][1], _m[1][2], _m[1][3],
		_m[2][0], _m[2][1], _m[2][2], _m[2][3],
		_m[3][0], _m[3][1], _m[3][2], _m[3][3]) {}

	static const Matrix4 ZERO;
	static const Matrix4 IDENTITY;
};

struct YAE_API Quaternion : public glm::quat
{
	Quaternion() {}
	Quaternion(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }
	Quaternion(const glm::quat& _q) : Quaternion(_q.x, _q.y, _q.z, _q.w) {}
	Quaternion(float _pitch, float _yaw, float _roll) : glm::quat(glm::vec3(_pitch, _yaw, _roll)) {}
	Quaternion(const Vector3& _eulerAngles) : Quaternion(_eulerAngles.x, _eulerAngles.y, _eulerAngles.z) {}

	Vector3 forward() const;
	Vector3 up() const;
	Vector3 right() const;

	Matrix4 toMatrix4() const;

	static const Quaternion IDENTITY;
};

YAE_API String toString(const Vector2& _v);
YAE_API String toString(const Vector3& _v);
YAE_API String toString(const Vector4& _v);

} // namespace yae
