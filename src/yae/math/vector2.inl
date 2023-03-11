namespace yae {

bool isZero(const Vector2& _v, float _threshold)
{
	return isZero(_v.x, _threshold) && isZero(_v.y, _threshold);
}

namespace vector2 {

float dot(const Vector2& _a, const Vector2& _b)
{
	return _a.x*_b.x + _a.y*_b.y;
}

} // namespace vector2
} // namespace yae