namespace yae {

bool isZero(const Vector3& _v, float _threshold)
{
	return isZero(_v.x, _threshold) && isZero(_v.y, _threshold) && isZero(_v.z, _threshold);
}

namespace vector3 {

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

} // namespace vector3
} // namespace yae
