#pragma once

#include <yae/types.h>

namespace yae {
namespace math {

inline bool isNan(const Matrix4& _m);
inline bool isFinite(const Matrix4& _m);

inline const float* data(const Matrix4& _m);
inline float* data(Matrix4& _m);

inline Vector3 translation(const Matrix4& _m);
inline Quaternion rotation(const Matrix4& _m);
inline Vector3 scale(const Matrix4& _m);
inline void decompose(const Matrix4& _m, Vector3& _translation, Quaternion& _rotation, Vector3& _scale);

inline Matrix4 translate(const Matrix4& _m, const Vector3& _translation);
inline Matrix4 rotate(const Matrix4& _m, const Quaternion& _rotation);
inline Matrix4 scale(const Matrix4& _m, const Vector3& _scale);
inline Matrix4 inverse(const Matrix4& _m);

inline Vector3 project(const Vector3& _worldPosition, const Matrix4& _view, const Matrix4& _projection, const Vector4& _viewport);
inline void unproject(const Vector2& _windowPosition, const Matrix4& _view, const Matrix4& _projection, const Vector4& _viewport, Vector3& _outRayOrigin, Vector3& _outRayDirection);
	
} // namespace math
} // namespace yae

#include "matrix4.inl"
