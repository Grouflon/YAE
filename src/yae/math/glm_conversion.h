#pragma once

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/matrix_float3x3.hpp>
#include <glm/ext/matrix_float4x4.hpp>

namespace yae {

inline glm::vec2 toGlm(const Vector2& _v) { return glm::vec2(_v.x, _v.y); }
inline glm::vec3 toGlm(const Vector3& _v) { return glm::vec3(_v.x, _v.y, _v.z); }
inline glm::vec4 toGlm(const Vector4& _v) { return glm::vec4(_v.x, _v.y, _v.z, _v.w); }
inline glm::quat toGlm(const Quaternion& _q) { return *(const glm::quat*)&_q; }
inline glm::mat3x3 toGlm(const Matrix3& _m) { return *(const glm::mat3x3*)&_m; }
inline glm::mat4x4 toGlm(const Matrix4& _m) { return *(const glm::mat4x4*)&_m; }

inline Vector2 toYae(const glm::vec2& _v) { return Vector2(_v.x, _v.y); }
inline Vector3 toYae(const glm::vec3& _v) { return Vector3(_v.x, _v.y, _v.z); }
inline Vector4 toYae(const glm::vec4& _v) { return Vector4(_v.x, _v.y, _v.z, _v.w); }
inline Quaternion toYae(const glm::quat& _q) { return *(const Quaternion*)&_q; }
inline Matrix3 toYae(const glm::mat3x3& _m) { return *(const Matrix3*)&_m; }
inline Matrix4 toYae(const glm::mat4x4& _m) { return *(const Matrix4*)&_m; }

} // namespace yae
