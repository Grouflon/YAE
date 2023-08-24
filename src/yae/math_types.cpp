#include "math_types.h"

#include <yae/math/glm_conversion.h>

#include <mirror/mirror.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace yae {

// - Quaternion -
Quaternion Quaternion::FromEuler(float _pitch, float _yaw, float _roll)
{
	return toYae(glm::quat(glm::vec3(_pitch, _yaw, _roll)));
}

Quaternion Quaternion::FromEuler(const Vector3& _radianAngles)
{
	return FromEuler(_radianAngles.x, _radianAngles.y, _radianAngles.z);
}

Quaternion Quaternion::FromAngleAxis(float _radianAngle, const Vector3& _axis)
{
	return toYae(glm::angleAxis(_radianAngle, toGlm(_axis)));
}

Quaternion Quaternion::FromMatrix3(const Matrix3& _m)
{
	return toYae(glm::quat_cast(toGlm(_m)));
}

Quaternion Quaternion::FromMatrix4(const Matrix4& _m)
{
	return toYae(glm::quat_cast(toGlm(_m)));
}

// Matrix3 static ctors
Matrix3 Matrix3::FromRotation(const Quaternion& _rotation)
{
	return toYae(glm::mat3x3(toGlm(_rotation)));
}

// Matrix4 static ctors
Matrix4 Matrix4::FromTranslation(const Vector3& _translation)
{
	return Matrix4(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		_translation.x, _translation.y, _translation.z, 1.f
	);
}

Matrix4 Matrix4::FromRotation(const Quaternion& _rotation)
{
	return Matrix4::FromMatrix3(Matrix3::FromRotation(_rotation));
}

Matrix4 Matrix4::FromScale(const Vector3& _scale)
{
	return Matrix4(
		_scale.x, 0.f, 0.f, 0.f,
		0.f, _scale.y, 0.f, 0.f,
		0.f, 0.f, _scale.z, 0.f,
		0.f, 0.f, 0.f, 1.f
	);
}

Matrix4 Matrix4::FromTransform(const Vector3& _position, const Quaternion& _rotation, const Vector3& _scale)
{
	Matrix4 t = Matrix4::FromTranslation(_position);
	Matrix4 r = Matrix4::FromRotation(_rotation);
	Matrix4 s = Matrix4::FromScale(_scale);
	return t * r * s;
}

Matrix4 Matrix4::FromTransform(const Transform& _transform)
{
	return FromTransform(_transform.position, _transform.rotation, _transform.scale);
}

Matrix4 Matrix4::FromPerspective(float _fov, float _aspectRatio, float _nearPlane, float _farPlane)
{
	return toYae(glm::perspectiveLH(_fov, _aspectRatio, _nearPlane, _farPlane));
}

Matrix4 Matrix4::FromLookAt(const Vector3& _center, const Vector3& _target, const Vector3& _up)
{
	return toYae(glm::lookAtLH(toGlm(_center), toGlm(_target), toGlm(_up)));
}

Matrix4 Matrix4::FromMatrix3(const Matrix3& _m)
{
	return Matrix4(
		_m.m[0][0], _m.m[0][1], _m.m[0][2], 0.f,
		_m.m[1][0], _m.m[1][1], _m.m[1][2], 0.f,
		_m.m[2][0], _m.m[2][1], _m.m[2][2], 0.f,
		0.f, 0.f, 0.f, 1.f
	);
}

} // namespace yae

MIRROR_CLASS(yae::Vector2, SerializeType = float, SerializeArraySize = 2)
(
	MIRROR_MEMBER(x);
	MIRROR_MEMBER(y);
);

MIRROR_CLASS(yae::Vector3, SerializeType = float, SerializeArraySize = 3)
(
	MIRROR_MEMBER(x);
	MIRROR_MEMBER(y);
	MIRROR_MEMBER(z);
);

MIRROR_CLASS(yae::Vector4, SerializeType = float, SerializeArraySize = 4)
(
	MIRROR_MEMBER(x);
	MIRROR_MEMBER(y);
	MIRROR_MEMBER(z);
	MIRROR_MEMBER(w);
);

MIRROR_CLASS(yae::Quaternion, SerializeType = float, SerializeArraySize = 4)
(
	MIRROR_MEMBER(x);
	MIRROR_MEMBER(y);
	MIRROR_MEMBER(z);
	MIRROR_MEMBER(w);
);

MIRROR_CLASS(yae::Matrix3)
(
	MIRROR_MEMBER(m);
);

MIRROR_CLASS(yae::Matrix4)
(
	MIRROR_MEMBER(m);
);

MIRROR_CLASS(yae::Transform)
(
	MIRROR_MEMBER(position);
	MIRROR_MEMBER(rotation);
	MIRROR_MEMBER(scale);
);
