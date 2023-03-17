#pragma once

#include <yae/types.h>

namespace yae {

inline String toString(const Vector2& _v);

namespace math {

inline bool isZero(const Vector2& _v, float _threshold = SMALL_NUMBER);

inline const float* data(const Vector2& _v);
inline float* data(Vector2& _v);

inline float lengthSquared(const Vector2& _v);
inline float length(const Vector2& _v);
inline Vector2 normalize(const Vector2& _v);
inline Vector2 safeNormalize(const Vector2& _v, float _threshold = SMALL_NUMBER);

inline float dot(const Vector2& _a, const Vector2& _b);

} // namespace math
} // namespace yae

#include "vector2.inl"
