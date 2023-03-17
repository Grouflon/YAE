namespace yae {
namespace math {

Matrix3 inverse(const Matrix3& _m)
{
	return toYae(glm::inverse(toGlm(_m)));
}

} // namespace math
} // namespace yae
