#pragma once

#include <yae/types.h>
#include <yae/math_types.h>

namespace yae {

inline bool isZero(const Vector4& _v, float _threshold = SMALL_NUMBER);

namespace vector4 {

inline float lengthSquared(const Vector4& _v);
inline float length(const Vector4& _v);
inline Vector4 normalize(const Vector4& _v);
inline Vector4 safeNormalize(const Vector4& _v, float _threshold = SMALL_NUMBER);

inline float dot(const Vector4& _a, const Vector4& _b);

} // namespace vector4
} // namespace yae

#include "vector4.inl"
