#pragma once

#include <yae/types.h>

namespace yae {
namespace matrix4 {

inline Matrix4 makeTranslation(const Vector3& _translation);
inline Matrix4 makeRotation(const Quaternion& _rotation);
inline Matrix4 makeScale(const Vector3& _scale);
inline Matrix4 makeTransform(const Vector3& _position, const Quaternion& _rotation, const Vector3& _scale);
inline Matrix4 makePerspective(float _fov, float _aspectRatio, float _nearPlane, float _farPlane);
inline Matrix4 makeFromMatrix3(const Matrix3& _m);

inline Matrix4 translate(const Matrix4& _m, const Vector3& _translation);
inline Matrix4 rotate(const Matrix4& _m, const Quaternion& _rotation);
inline Matrix4 scale(const Matrix4& _m, const Vector3& _scale);
inline Matrix4 inverse(const Matrix4& _m);
	
} // namespace matrix4
} // namespace yae

#include "matrix4.inl"
