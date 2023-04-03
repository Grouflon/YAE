namespace yae {
namespace math {

bool isNan(const Matrix3& _m)
{
	for (u32 i = 0; i < 9; ++i)
	{
		if (isNan(data(_m)[i]))
			return true;
	}
	return false;
}

bool isFinite(const Matrix3& _m)
{
	for (u32 i = 0; i < 9; ++i)
	{
		if (isFinite(data(_m)[i]))
			return true;
	}
	return false;
}

Matrix3 inverse(const Matrix3& _m)
{
	return toYae(glm::inverse(toGlm(_m)));
}

} // namespace math
} // namespace yae
