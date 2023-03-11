namespace yae {
namespace matrix3 {

Matrix3 makeRotation(const Quaternion& _rotation)
{
	// Copied from glm: glm/gtc/quaternion.inl:49
	Matrix3 result;
	float qxx = _rotation.x * _rotation.x;
	float qyy = _rotation.y * _rotation.y;
	float qzz = _rotation.z * _rotation.z;
	float qxz = _rotation.x * _rotation.z;
	float qxy = _rotation.x * _rotation.y;
	float qyz = _rotation.y * _rotation.z;
	float qwx = _rotation.w * _rotation.x;
	float qwy = _rotation.w * _rotation.y;
	float qwz = _rotation.w * _rotation.z;

	result.m[0][0] = 1.f - 2.f * (qyy +  qzz);
	result.m[0][1] = 2.f * (qxy + qwz);
	result.m[0][2] = 2.f * (qxz - qwy);

	result.m[1][0] = 2.f * (qxy - qwz);
	result.m[1][1] = 1.f - 2.f * (qxx +  qzz);
	result.m[1][2] = 2.f * (qyz + qwx);

	result.m[2][0] = 2.f * (qxz + qwy);
	result.m[2][1] = 2.f * (qyz - qwx);
	result.m[2][2] = 1.f - 2.f * (qxx +  qyy);
	return result;
}

Matrix3 inverse(const Matrix3& _m)
{
	// https://stackoverflow.com/questions/983999/simple-3x3-matrix-inverse-code-c
	// computes the inverse of a matrix m
	float det = _m.m[0][0] * (_m.m[1][1] * _m.m[2][2] - _m.m[2][1] * _m.m[1][2]) -
	             _m.m[0][1] * (_m.m[1][0] * _m.m[2][2] - _m.m[1][2] * _m.m[2][0]) +
	             _m.m[0][2] * (_m.m[1][0] * _m.m[2][1] - _m.m[1][1] * _m.m[2][0]);

	float invdet = 1.f / det;

	Matrix3 result; // inverse of matrix m
	result.m[0][0] = (_m.m[1][1] * _m.m[2][2] - _m.m[2][1] * _m.m[1][2]) * invdet;
	result.m[0][1] = (_m.m[0][2] * _m.m[2][1] - _m.m[0][1] * _m.m[2][2]) * invdet;
	result.m[0][2] = (_m.m[0][1] * _m.m[1][2] - _m.m[0][2] * _m.m[1][1]) * invdet;
	result.m[1][0] = (_m.m[1][2] * _m.m[2][0] - _m.m[1][0] * _m.m[2][2]) * invdet;
	result.m[1][1] = (_m.m[0][0] * _m.m[2][2] - _m.m[0][2] * _m.m[2][0]) * invdet;
	result.m[1][2] = (_m.m[1][0] * _m.m[0][2] - _m.m[0][0] * _m.m[1][2]) * invdet;
	result.m[2][0] = (_m.m[1][0] * _m.m[2][1] - _m.m[2][0] * _m.m[1][1]) * invdet;
	result.m[2][1] = (_m.m[2][0] * _m.m[0][1] - _m.m[0][0] * _m.m[2][1]) * invdet;
	result.m[2][2] = (_m.m[0][0] * _m.m[1][1] - _m.m[1][0] * _m.m[0][1]) * invdet;
	return result;
}

} // namespace matrix3
} // namespace yae
