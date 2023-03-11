#pragma once

namespace yae {

inline bool isZero(const Vector2& _v, float _threshold = SMALL_NUMBER);

namespace vector2 {

inline float dot(const Vector2& _a, const Vector2& _b);

} // namespace vector2
} // namespace yae

#include "vector2.inl"
