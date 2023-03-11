#pragma once

#include <yae/types.h>

namespace yae {

inline bool isZero(const Vector3& _v, float _threshold = SMALL_NUMBER);
	
namespace vector3 {

inline float lengthSquared(const Vector3& _v);
inline float length(const Vector3& _v);
inline Vector3 normalize(const Vector3& _v);
inline Vector3 safeNormalize(const Vector3& _v, float _threshold = SMALL_NUMBER);

inline float dot(const Vector3& _a, const Vector3& _b);
inline Vector3 cross(const Vector3& _a, const Vector3& _b);

} // namespace vector3
} // namespace yae

#include "vector3.inl"
