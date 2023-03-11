#pragma once

#include <yae/types.h>

namespace yae {
namespace matrix3 {

inline Matrix3 makeRotation(const Quaternion& _rotation);

inline Matrix3 inverse(const Matrix3& _m);
	
} // namespace matrix3
} // namespace yae

#include "matrix3.inl"
