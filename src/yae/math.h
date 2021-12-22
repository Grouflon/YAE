#pragma once

#include <yae/types.h>
#include <yae/math_types.h>

namespace yae {

YAELIB_API i8     abs(i8 _value);
YAELIB_API i16    abs(i16 _value);
YAELIB_API i32    abs(i32 _value);
YAELIB_API i64    abs(i64 _value);
YAELIB_API float  abs(float _value);
YAELIB_API double abs(double _value);

YAELIB_API u8     min(u8 _a, u8 _b);
YAELIB_API u16    min(u16 _a, u16 _b);
YAELIB_API u32    min(u32 _a, u32 _b);
YAELIB_API u64    min(u64 _a, u64 _b);
YAELIB_API i8     min(i8 _a, i8 _b);
YAELIB_API i16    min(i16 _a, i16 _b);
YAELIB_API i32    min(i32 _a, i32 _b);
YAELIB_API i64    min(i64 _a, i64 _b);
YAELIB_API float  min(float _a, float _b);
YAELIB_API double min(double _a, double _b);

YAELIB_API u8     max(u8 _a, u8 _b);
YAELIB_API u16    max(u16 _a, u16 _b);
YAELIB_API u32    max(u32 _a, u32 _b);
YAELIB_API u64    max(u64 _a, u64 _b);
YAELIB_API i8     max(i8 _a, i8 _b);
YAELIB_API i16    max(i16 _a, i16 _b);
YAELIB_API i32    max(i32 _a, i32 _b);
YAELIB_API i64    max(i64 _a, i64 _b);
YAELIB_API float  max(float _a, float _b);
YAELIB_API double max(double _a, double _b);

YAELIB_API float clamp(float _value, float _min, float _max);

YAELIB_API float mod(float _a, float _b);
YAELIB_API float positiveMod(float _a, float _b);

YAELIB_API bool isZero(float _value, float _threshold = SMALL_NUMBER);
YAELIB_API bool isZero(double _value, double _threshold = SMALL_NUMBER);
YAELIB_API bool isZero(const Vector2& _v, float _threshold = SMALL_NUMBER);
YAELIB_API bool isZero(const Vector3& _v, float _threshold = SMALL_NUMBER);
YAELIB_API bool isZero(const Vector4& _v, float _threshold = SMALL_NUMBER);

YAELIB_API Vector2 normalize(const Vector2& _v);
YAELIB_API Vector3 normalize(const Vector3& _v);
YAELIB_API Vector4 normalize(const Vector4& _v);
YAELIB_API Vector2 safeNormalize(const Vector2& _v, float _threshold = SMALL_NUMBER);
YAELIB_API Vector3 safeNormalize(const Vector3& _v, float _threshold = SMALL_NUMBER);
YAELIB_API Vector4 safeNormalize(const Vector4& _v, float _threshold = SMALL_NUMBER);

YAELIB_API Matrix4 inverse(const Matrix4& _m);

YAELIB_API Matrix4 toMatrix4(const Quaternion& _q);
YAELIB_API Matrix4 makeTransformMatrix(const Vector3& _position, const Quaternion& _rotation, const Vector3& _scale);

} // namespace yae
