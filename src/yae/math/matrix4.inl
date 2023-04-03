#include <yae/math/matrix3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/ext/matrix_projection.hpp>

namespace yae {
namespace math {

bool isNan(const Matrix4& _m)
{
	for (u32 i = 0; i < 16; ++i)
	{
		if (isNan(data(_m)[i]))
			return true;
	}
	return false;
}

bool isFinite(const Matrix4& _m)
{
	for (u32 i = 0; i < 16; ++i)
	{
		if (isFinite(data(_m)[i]))
			return true;
	}
	return false;
}

const float* data(const Matrix4& _m)
{
	return (const float*)&_m;
}

float* data(Matrix4& _m)
{
	return (float*)&_m;
}

Vector3 translation(const Matrix4& _m)
{
	glm::vec3 scale, translation, skew;
	glm::vec4 perspective;
	glm::quat rotation;
	glm::decompose(toGlm(_m), scale, rotation, translation, skew, perspective);
	return toYae(translation);
}

Quaternion rotation(const Matrix4& _m)
{
	glm::vec3 scale, translation, skew;
	glm::vec4 perspective;
	glm::quat rotation;
	glm::decompose(toGlm(_m), scale, rotation, translation, skew, perspective);
	return toYae(rotation);
}

Vector3 scale(const Matrix4& _m)
{
	glm::vec3 scale, translation, skew;
	glm::vec4 perspective;
	glm::quat rotation;
	glm::decompose(toGlm(_m), scale, rotation, translation, skew, perspective);
	return toYae(scale);
}

void decompose(const Matrix4& _m, Vector3& _translation, Quaternion& _rotation, Vector3& _scale)
{
	glm::vec3 scale, translation, skew;
	glm::vec4 perspective;
	glm::quat rotation;
	glm::decompose(toGlm(_m), scale, rotation, translation, skew, perspective);
	_translation = toYae(translation);
	_rotation = toYae(rotation);
	_scale = toYae(scale);
}

Matrix4 translate(const Matrix4& _m, const Vector3& _translation)
{
	return _m * Matrix4::FromTranslation(_translation);
}

Matrix4 rotate(const Matrix4& _m, const Quaternion& _rotation)
{
	return _m * Matrix4::FromRotation(_rotation);
}

Matrix4 scale(const Matrix4& _m, const Vector3& _scale)
{
	return _m * Matrix4::FromScale(_scale);
}

Matrix4 inverse(const Matrix4& _m)
{
	return toYae(glm::inverse(toGlm(_m)));
}

Vector3 project(const Vector3& _worldPosition, const Matrix4& _view, const Matrix4& _projection, const Vector4& _viewport)
{
	return toYae(glm::project(
		toGlm(_worldPosition),
		toGlm(_view),
		toGlm(_projection),
		toGlm(_viewport)
	));
}

void unproject(const Vector2& _windowPosition, const Matrix4& _view, const Matrix4& _projection, const Vector4& _viewport, Vector3& _outRayOrigin, Vector3& _outRayDirection)
{
	glm::vec3 near = glm::unProject(
		glm::vec3(_windowPosition.x, _windowPosition.y, -1.f),
		toGlm(_view),
		toGlm(_projection),
		toGlm(_viewport)
	);
	glm::vec3 far = glm::unProject(
		glm::vec3(_windowPosition.x, _windowPosition.y, 1.f),
		toGlm(_view),
		toGlm(_projection),
		toGlm(_viewport)
	);

	_outRayOrigin = toYae(near);
	_outRayDirection = toYae(glm::normalize(far - near));
}

} // namespace math
} // namespace yae
