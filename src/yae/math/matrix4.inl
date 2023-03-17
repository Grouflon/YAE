#include <yae/math/matrix3.h>

#include <glm/gtc/matrix_transform.hpp>

namespace yae {
namespace math {

const float* data(const Matrix4& _m)
{
	return (const float*)&_m;
}

float* data(Matrix4& _m)
{
	return (float*)&_m;
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
	
} // namespace math
} // namespace yae
