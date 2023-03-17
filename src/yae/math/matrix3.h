#pragma once

#include <yae/types.h>

namespace yae {
namespace math {

inline const float* data(const Matrix3& _m) { return (const float*)&_m; }
inline float* data(Matrix3& _m) { return (float*)&_m; }

inline Matrix3 inverse(const Matrix3& _m);
	
} // namespace math
} // namespace yae

#include "matrix3.inl"
