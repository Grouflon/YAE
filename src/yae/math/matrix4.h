#pragma once

#include <yae/types.h>

namespace yae {
namespace math {

inline const float* data(const Matrix4& _m);
inline float* data(Matrix4& _m);

inline Matrix4 translate(const Matrix4& _m, const Vector3& _translation);
inline Matrix4 rotate(const Matrix4& _m, const Quaternion& _rotation);
inline Matrix4 scale(const Matrix4& _m, const Vector3& _scale);
inline Matrix4 inverse(const Matrix4& _m);
	
} // namespace math
} // namespace yae

#include "matrix4.inl"
