#pragma once

#include <yae/types.h>
#include <yae/math_types.h>

namespace yae {

inline String toString(const Vector4& _v);

namespace math {

inline bool isZero(const Vector4& _v, float _threshold = SMALL_NUMBER);

inline const float* data(const Vector4& _v);
inline float* data(Vector4& _v);

inline Vector3 xyz(const Vector4& _v);

inline float lengthSquared(const Vector4& _v);
inline float length(const Vector4& _v);
inline Vector4 normalize(const Vector4& _v);
inline Vector4 safeNormalize(const Vector4& _v, float _threshold = SMALL_NUMBER);

inline float dot(const Vector4& _a, const Vector4& _b);

} // namespace math
} // namespace yae

#include "vector4.inl"
