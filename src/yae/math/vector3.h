#pragma once

#include <yae/types.h>

namespace yae {
namespace math {

inline bool isZero(const Vector3& _v, float _threshold = SMALL_NUMBER);
inline bool isNan(const Vector3& _v);
inline bool isFinite(const Vector3& _v);

inline const float* data(const Vector3& _v);
inline float* data(Vector3& _v);

inline Vector2 xy(const Vector3& _v);
inline Vector2 xz(const Vector3& _v);
inline Vector2 yx(const Vector3& _v);
inline Vector2 yz(const Vector3& _v);
inline Vector2 zx(const Vector3& _v);
inline Vector2 zy(const Vector3& _v);

inline float lengthSquared(const Vector3& _v);
inline float length(const Vector3& _v);
inline Vector3 normalize(const Vector3& _v);
inline Vector3 safeNormalize(const Vector3& _v, float _threshold = SMALL_NUMBER);

inline float dot(const Vector3& _a, const Vector3& _b);
inline Vector3 cross(const Vector3& _a, const Vector3& _b);

} // namespace math
} // namespace yae

#include "vector3.inl"
