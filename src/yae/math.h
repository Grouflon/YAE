#pragma once

#include <yae/math_types.h>

namespace yae {

Vector2 normalize(const Vector2& _v) { return glm::normalize(_v); }
Vector3 normalize(const Vector3& _v) { return glm::normalize(_v); }
Vector4 normalize(const Vector4& _v) { return glm::normalize(_v); }

Mat4 inverse(const Mat4& _m) { return glm::inverse(_m); }

} // namespace yae
