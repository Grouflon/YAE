#include <yae/math.h>

#include <cmath>

namespace yae {
namespace math {

bool isZero(const Vector4& _v, float _threshold)
{
	return isZero(_v.x, _threshold) && isZero(_v.y, _threshold) && isZero(_v.z, _threshold) && isZero(_v.w, _threshold);
}

const float* data(const Vector4& _v)
{
	return (const float*)&_v;
}

float* data(Vector4& _v)
{
	return (float*)&_v;
}

Vector3 xyz(const Vector4& _v)
{
	return Vector3(_v.x, _v.y, _v.z);
}

float lengthSquared(const Vector4& _v)
{
	return _v.x*_v.x + _v.y*_v.y + _v.z*_v.z + _v.w*_v.w;
}

float length(const Vector4& _v)
{
	return std::sqrt(lengthSquared(_v));
}

Vector4 normalize(const Vector4& _v)
{
	float l = length(_v);
	YAE_ASSERT(l > SMALL_NUMBER);
	return _v / l;
}

Vector4 safeNormalize(const Vector4& _v, float _threshold)
{
	float l = length(_v);
	if (l < _threshold)
		return Vector4(0.f);
	return _v / l;
}

float dot(const Vector4& _a, const Vector4& _b)
{
	return _a.x*_b.x + _a.y*_b.y + _a.z*_b.z + _a.w*_b.w;
}

} // namespace math
} // namespace yae
