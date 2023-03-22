#pragma once

#include <yae/types.h>
#include <yae/math.h>

namespace yae {
namespace math {

template <>
inline bool isEqual(const Quaternion& _a, const Quaternion& _b, float _threshold);
inline bool isIdentical(const Quaternion& _a, const Quaternion& _b, float _threshold = SMALL_NUMBER);

inline const float* data(const Quaternion& _q);
inline float* data(Quaternion& _q);

inline float lengthSquared(const Quaternion& _q);
inline float length(const Quaternion& _q);
inline Quaternion normalize(const Quaternion& _q);
inline Quaternion safeNormalize(const Quaternion& _q, float _threshold = SMALL_NUMBER);

inline float dot(const Quaternion& _a, const Quaternion& _b);
inline Quaternion conjugate(const Quaternion& _q);
inline Quaternion inverse(const Quaternion& _q);

inline Vector3 forward(const Quaternion& _q);
inline Vector3 right(const Quaternion& _q);
inline Vector3 up(const Quaternion& _q);

inline float angle(const Quaternion& _q);
inline Vector3 axis(const Quaternion& _q);

inline float roll(const Quaternion& _q);
inline float pitch(const Quaternion& _q);
inline float yaw(const Quaternion& _q);
inline Vector3 euler(const Quaternion& _q, bool _preventFlip = true);

inline Vector3 rotate(const Quaternion& _q, const Vector3& _v);
inline Quaternion rotationBetween(const Vector3& _start, const Vector3& _dest); // Rotation required to turn _start into _dest

} // namespace math
} // namespace yae

#include "quaternion.inl"
