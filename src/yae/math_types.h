#pragma once

#include <core/types.h>

namespace yae {

/*
Super useful documentation on matrices and vectors:
- https://web.archive.org/web/20130529234948/http://seanmiddleditch.com/journal/2012/08/matrices-handedness-pre-and-post-multiplication-row-vs-column-major-and-notations/
- https://seanmiddleditch.github.io/matrices-handedness-pre-and-post-multiplication-row-vs-column-major-and-notations/
*/

/*
Matrix Notation:   Column major
Matrix Layout:     Column major
Product order:     Post-multiplication (Means that vectors are column vectors)
Coordinate System: Left-Handed
*/


// CONSTANTS
#define PI (3.14159265358979323846)
#define R2D (180.f / PI)
#define D2R (PI / 180.f)

// TYPES
struct Vector2;
struct Vector3;
struct Vector4;
struct Quaternion;
struct Matrix3;
struct Matrix4;
struct Transform;

// - Vector2 -
struct YAE_API Vector2
{
	float x;
	float y;

	// Ctors
	Vector2(); // leaves uninitialized
	Vector2(float _value);
	Vector2(float _x, float _y);

	// Constants
	static Vector2 ZERO();
	static Vector2 ONE();
	static Vector2 RIGHT();
	static Vector2 UP();

	// Operators
	// -- Component accesses --
	float& operator[](size_t _i);
	const float& operator[](size_t _i) const;

	// -- Unary arithmetic operators --
	Vector2& operator+=(float _s);
	Vector2& operator+=(const Vector2& _v);
	Vector2& operator-=(float _s);
	Vector2& operator-=(const Vector2& _v);
	Vector2& operator*=(float _s);
	Vector2& operator*=(const Vector2& _v);
	Vector2& operator/=(float _s);
	Vector2& operator/=(const Vector2& _v);
};
// -- Unary operators --
Vector2 operator+(const Vector2& _v);
Vector2 operator-(const Vector2& _v);

// -- Binary operators --
Vector2 operator+(const Vector2& _v, float _s);
Vector2 operator+(float _s, const Vector2& _v);
Vector2 operator+(const Vector2& _v1, const Vector2& _v2);
Vector2 operator-(const Vector2& _v, float _s);
Vector2 operator-(float _s, const Vector2& _v);
Vector2 operator-(const Vector2& _v1, const Vector2& _v2);
Vector2 operator*(const Vector2& _v, float _s);
Vector2 operator*(float _s, const Vector2& _v);
Vector2 operator*(const Vector2& _v1, const Vector2& _v2);
Vector2 operator/(const Vector2& _v, float _s);
Vector2 operator/(float _s, const Vector2& _v);
Vector2 operator/(const Vector2& _v1, const Vector2& _v2);

// -- Boolean operators --
bool operator==(const Vector2& _v1, const Vector2& _v2);
bool operator!=(const Vector2& _v1, const Vector2& _v2);

// - Vector3 -
struct YAE_API Vector3
{
	float x;
	float y;
	float z;

	// Ctors
	Vector3();  // leaves uninitialized
	Vector3(float _value);
	Vector3(float _x, float _y, float _z);
	Vector3(const Vector2& _xy, float _z = 0.f);

	// Constants
	static Vector3 ZERO();
	static Vector3 ONE();
	static Vector3 RIGHT();
	static Vector3 UP();
	static Vector3 FORWARD();

	// Operators
	// -- Component accesses --
	float& operator[](size_t _i);
	const float& operator[](size_t _i) const;

	// -- Unary arithmetic operators --
	Vector3& operator+=(float _s);
	Vector3& operator+=(const Vector3& _v);
	Vector3& operator-=(float _s);
	Vector3& operator-=(const Vector3& _v);
	Vector3& operator*=(float _s);
	Vector3& operator*=(const Vector3& _v);
	Vector3& operator/=(float _s);
	Vector3& operator/=(const Vector3& _v);
};
// -- Unary operators --
Vector3 operator+(const Vector3& _v);
Vector3 operator-(const Vector3& _v);

// -- Binary operators --
Vector3 operator+(const Vector3& _v, float _s);
Vector3 operator+(float _s, const Vector3& _v);
Vector3 operator+(const Vector3& _v1, const Vector3& _v2);
Vector3 operator-(const Vector3& _v, float _s);
Vector3 operator-(float _s, const Vector3& _v);
Vector3 operator-(const Vector3& _v1, const Vector3& _v2);
Vector3 operator*(const Vector3& _v, float _s);
Vector3 operator*(float _s, const Vector3& _v);
Vector3 operator*(const Vector3& _v1, const Vector3& _v2);
Vector3 operator/(const Vector3& _v, float _s);
Vector3 operator/(float _s, const Vector3& _v);
Vector3 operator/(const Vector3& _v1, const Vector3& _v2);

// -- Boolean operators --
bool operator==(const Vector3& _v1, const Vector3& _v2);
bool operator!=(const Vector3& _v1, const Vector3& _v2);

// - Vector4 -
struct YAE_API Vector4
{
	float x;
	float y;
	float z;
	float w;

	// Ctors
	Vector4();  // leaves uninitialized
	Vector4(float _value);
	Vector4(float _x, float _y, float _z, float _w);
	Vector4(const Vector2& _xy, float _z = 0.f, float _w = 0.f);
	Vector4(const Vector3& _xyz, float _w = 0.f);

	// Constants
	static Vector4 ZERO();
	static Vector4 ONE();

	// Operators
	// -- Component accesses --
	float& operator[](size_t _i);
	const float& operator[](size_t _i) const;

	// -- Unary arithmetic operators --
	Vector4& operator+=(float _s);
	Vector4& operator+=(const Vector4& _v);
	Vector4& operator-=(float _s);
	Vector4& operator-=(const Vector4& _v);
	Vector4& operator*=(float _s);
	Vector4& operator*=(const Vector4& _v);
	Vector4& operator/=(float _s);
	Vector4& operator/=(const Vector4& _v);
};
// -- Unary operators --
Vector4 operator+(const Vector4& _v);
Vector4 operator-(const Vector4& _v);

// -- Binary operators --
Vector4 operator+(const Vector4& _v, float _s);
Vector4 operator+(float _s, const Vector4& _v);
Vector4 operator+(const Vector4& _v1, const Vector4& _v2);
Vector4 operator-(const Vector4& _v, float _s);
Vector4 operator-(float _s, const Vector4& _v);
Vector4 operator-(const Vector4& _v1, const Vector4& _v2);
Vector4 operator*(const Vector4& _v, float _s);
Vector4 operator*(float _s, const Vector4& _v);
Vector4 operator*(const Vector4& _v1, const Vector4& _v2);
Vector4 operator/(const Vector4& _v, float _s);
Vector4 operator/(float _s, const Vector4& _v);
Vector4 operator/(const Vector4& _v1, const Vector4& _v2);

// -- Boolean operators --
bool operator==(const Vector4& _v1, const Vector4& _v2);
bool operator!=(const Vector4& _v1, const Vector4& _v2);

// - Quaternion -
struct YAE_API Quaternion
{
	float x;
	float y;
	float z;
	float w;

	// Ctors
	Quaternion();  // leaves uninitialized
	Quaternion(float _value);
	Quaternion(float _x, float _y, float _z, float _w);
	static Quaternion FromEuler(float _pitch, float _yaw, float _roll);
	static Quaternion FromEuler(const Vector3& _radianAngles);
	static Quaternion FromAngleAxis(float _radianAngle, const Vector3& _axis);
	static Quaternion FromMatrix3(const Matrix3& _m);
	static Quaternion FromMatrix4(const Matrix4& _m);

	// Constants
	static Quaternion IDENTITY();

	// Operators
	// -- Component accesses --
	float& operator[](size_t _i);
	const float& operator[](size_t _i) const;

	// -- Unary arithmetic operators --
	Quaternion& operator*=(float _s);
	Quaternion& operator*=(const Quaternion& _q);
	Quaternion& operator/=(float _s);
};
// -- Binary operators --
Quaternion operator*(const Quaternion& _q, float _s);
Quaternion operator*(const Quaternion& _q1, const Quaternion& _q2);
Vector3 operator*(const Quaternion& _q, const Vector3& _v);
Quaternion operator/(const Quaternion& _q, float _s);

// -- Boolean operators --
bool operator==(const Quaternion& _q1, const Quaternion& _q2);
bool operator!=(const Quaternion& _q1, const Quaternion& _q2);

// - Matrix3 -
struct YAE_API Matrix3
{
	Vector3 m[3];

	// Ctors
	Matrix3(); // leaves uninitialized
	Matrix3(float _value);
	Matrix3(float _m00, float _m01, float _m02,
			float _m10, float _m11, float _m12,
			float _m20, float _m21, float _m22);
	Matrix3(const Vector3& _m0,
			const Vector3& _m1,
			const Vector3& _m2);
	static Matrix3 FromRotation(const Quaternion& _rotation);

	// Constants
	static Matrix3 ZERO();
	static Matrix3 IDENTITY();

	// Operators
	// -- Component accesses --
	Vector3& operator[](size_t _i);
	const Vector3& operator[](size_t _i) const;

	// -- Unary arithmetic operators --
	Matrix3& operator+=(float _s);
	Matrix3& operator+=(const Matrix3& _m);
	Matrix3& operator-=(float _s);
	Matrix3& operator-=(const Matrix3& _m);
	Matrix3& operator*=(float _s);
	Matrix3& operator*=(const Matrix3& _m);
	Matrix3& operator/=(float _s);
	Matrix3& operator/=(const Matrix3& _m);
};
// -- Unary operators --
Matrix3 operator+(const Matrix3& _m);
Matrix3 operator-(const Matrix3& _m);

// -- Binary operators --
Matrix3 operator+(const Matrix3& _m, float _s);
Matrix3 operator+(float _s, const Matrix3& _m);
Matrix3 operator+(const Matrix3& _m1, const Matrix3& _m2);
Matrix3 operator-(const Matrix3& _m, float _s);
Matrix3 operator-(float _s, const Matrix3& _m);
Matrix3 operator-(const Matrix3& _m1, const Matrix3& _m2);
Matrix3 operator*(const Matrix3& _m, float _s);
Matrix3 operator*(float _s, const Matrix3& _m);
Matrix3 operator*(const Matrix3& _m1, const Matrix3& _m2);
Vector3 operator*(const Matrix3& _m, const Vector3& _v);
Vector3 operator*(const Matrix3& _m, const Vector2& _v);
Matrix3 operator/(const Matrix3& _m, float _s);
Matrix3 operator/(float _s, const Matrix3& _m);
Matrix3 operator/(const Matrix3& _m1, const Matrix3& _m2);

// -- Boolean operators --
bool operator==(const Matrix3& _m1, const Matrix3& _m2);
bool operator!=(const Matrix3& _m1, const Matrix3& _m2);

// - Matrix 4 -
struct YAE_API Matrix4
{
	Vector4 m[4];

	// Ctors
	Matrix4(); // leaves uninitialized
	Matrix4(float _value);
	Matrix4(float _m00, float _m01, float _m02, float _m03,
			float _m10, float _m11, float _m12, float _m13,
			float _m20, float _m21, float _m22, float _m23,
			float _m30, float _m31, float _m32, float _m33);
	Matrix4(const Vector4& _m0,
			const Vector4& _m1,
			const Vector4& _m2,
			const Vector4& _m3);
	static Matrix4 FromTranslation(const Vector3& _translation);
	static Matrix4 FromRotation(const Quaternion& _rotation);
	static Matrix4 FromScale(const Vector3& _scale);
	static Matrix4 FromTransform(const Vector3& _position, const Quaternion& _rotation, const Vector3& _scale);
	static Matrix4 FromTransform(const Transform& _transform);
	static Matrix4 FromPerspective(float _fov, float _aspectRatio, float _nearPlane, float _farPlane);
	static Matrix4 FromLookAt(const Vector3& _center, const Vector3& _target, const Vector3& _up = Vector3::UP());
	static Matrix4 FromMatrix3(const Matrix3& _m);

	// Constants
	static Matrix4 ZERO();
	static Matrix4 IDENTITY();

	// Operators
	// -- Component accesses --
	Vector4& operator[](size_t _i);
	const Vector4& operator[](size_t _i) const;

	// -- Unary arithmetic operators --
	Matrix4& operator+=(float _s);
	Matrix4& operator+=(const Matrix4& _m);
	Matrix4& operator-=(float _s);
	Matrix4& operator-=(const Matrix4& _m);
	Matrix4& operator*=(float _s);
	Matrix4& operator*=(const Matrix4& _m);
	Matrix4& operator/=(float _s);
	Matrix4& operator/=(const Matrix4& _m);
};
// -- Unary operators --
Matrix4 operator+(const Matrix4& _m);
Matrix4 operator-(const Matrix4& _m);

// -- Binary operators --
Matrix4 operator+(const Matrix4& _m, float _s);
Matrix4 operator+(float _s, const Matrix4& _m);
Matrix4 operator+(const Matrix4& _m1, const Matrix4& _m2);
Matrix4 operator-(const Matrix4& _m, float _s);
Matrix4 operator-(float _s, const Matrix4& _m);
Matrix4 operator-(const Matrix4& _m1, const Matrix4& _m2);
Matrix4 operator*(const Matrix4& _m, float _s);
Matrix4 operator*(float _s, const Matrix4& _m);
Matrix4 operator*(const Matrix4& _m1, const Matrix4& _m2);
Vector4 operator*(const Matrix4& _m, const Vector4& _v);
Vector3 operator*(const Matrix4& _m, const Vector3& _v);
Vector2 operator*(const Matrix4& _m, const Vector2& _v);
Matrix4 operator/(const Matrix4& _m, float _s);
Matrix4 operator/(float _s, const Matrix4& _m);
Matrix4 operator/(const Matrix4& _m1, const Matrix4& _m2);

// -- Boolean operators --
bool operator==(const Matrix4& _m1, const Matrix4& _m2);
bool operator!=(const Matrix4& _m1, const Matrix4& _m2);


// - Transform
struct YAE_API Transform
{
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;

	// Ctors
	Transform(); // leaves uninitialized
	Transform(const Vector3& _position,
			  const Quaternion& _rotation = Quaternion::IDENTITY(),
			  const Vector3& _scale = Vector3::ONE());

	// Constants
	static Transform IDENTITY();
};

// -- Binary operators --
Transform operator*(const Transform& _t1, const Transform& _t2);
Vector4 operator*(const Transform& _t, const Vector4& _v);
Vector3 operator*(const Transform& _t, const Vector3& _v);
Vector2 operator*(const Transform& _t, const Vector2& _v);
Quaternion operator*(const Transform& _t, const Quaternion& _q);

// -- Boolean operators --
bool operator==(const Transform& _t1, const Transform& _t2);
bool operator!=(const Transform& _t1, const Transform& _t2);

} // namespace yae

#include "math_types.inl"
