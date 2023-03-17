namespace yae {

String toString(const Vector2& _v)
{
	return string::format("{%.2f, %.2f}", _v.x, _v.y);
}

namespace math {

bool isZero(const Vector2& _v, float _threshold)
{
	return isZero(_v.x, _threshold) && isZero(_v.y, _threshold);
}

const float* data(const Vector2& _v)
{
	return (const float*)&_v;
}

float* data(Vector2& _v)
{
	return (float*)&_v;
}

float lengthSquared(const Vector2& _v)
{
	return _v.x*_v.x + _v.y*_v.y;
}

float length(const Vector2& _v)
{
	return std::sqrt(lengthSquared(_v));
}

Vector2 normalize(const Vector2& _v)
{
	float l = length(_v);
	YAE_ASSERT(l > SMALL_NUMBER);
	return _v / l;
}

Vector2 safeNormalize(const Vector2& _v, float _threshold)
{
	float l = length(_v);
	if (l < _threshold)
		return Vector2(0.f);
	return _v / l;
}

float dot(const Vector2& _a, const Vector2& _b)
{
	return _a.x*_b.x + _a.y*_b.y;
}

} // namespace math
} // namespace yae
