namespace yae {
namespace math {

bool isZero(const Vector3& _v, float _threshold)
{
	return isZero(_v.x, _threshold) && isZero(_v.y, _threshold) && isZero(_v.z, _threshold);
}

bool isNan(const Vector3& _v)
{
	return isNan(_v.x) || isNan(_v.y) || isNan(_v.z);
}

bool isFinite(const Vector3& _v)
{
	return isFinite(_v.x) || isFinite(_v.y) || isFinite(_v.z);
}

const float* data(const Vector3& _v)
{
	return (const float*)&_v;
}

float* data(Vector3& _v)
{
	return (float*)&_v;
}

Vector2 xy(const Vector3& _v)
{
	return Vector2(_v.x, _v.y);
}

Vector2 xz(const Vector3& _v)
{
	return Vector2(_v.x, _v.z);
}

Vector2 yx(const Vector3& _v)
{
	return Vector2(_v.y, _v.x);
}

Vector2 yz(const Vector3& _v)
{
	return Vector2(_v.y, _v.z);
}

Vector2 zx(const Vector3& _v)
{
	return Vector2(_v.z, _v.x);
}

Vector2 zy(const Vector3& _v)
{
	return Vector2(_v.z, _v.y);
}

float lengthSquared(const Vector3& _v)
{
	return _v.x*_v.x + _v.y*_v.y + _v.z*_v.z;
}

float length(const Vector3& _v)
{
	return std::sqrt(lengthSquared(_v));
}

Vector3 normalize(const Vector3& _v)
{
	float l = length(_v);
	YAE_ASSERT(l > SMALL_NUMBER);
	return _v / l;
}

Vector3 safeNormalize(const Vector3& _v, float _threshold)
{
	float l = length(_v);
	if (l < _threshold)
		return Vector3(0.f);
	return _v / l;
}

float dot(const Vector3& _a, const Vector3& _b)
{
	return _a.x*_b.x + _a.y*_b.y + _a.z*_b.z;
}

Vector3 cross(const Vector3& _a, const Vector3& _b)
{
	return Vector3(
		_a.y * _b.z - _b.y * _a.z,
		_a.z * _b.x - _b.z * _a.x,
		_a.x * _b.y - _b.x * _a.y
	);
}

} // namespace math
} // namespace yae
