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
	// https://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
	float A2323 = _m.m[2][2] * _m.m[3][3] - _m.m[2][3] * _m.m[3][2];
	float A1323 = _m.m[2][1] * _m.m[3][3] - _m.m[2][3] * _m.m[3][1];
	float A1223 = _m.m[2][1] * _m.m[3][2] - _m.m[2][2] * _m.m[3][1];
	float A0323 = _m.m[2][0] * _m.m[3][3] - _m.m[2][3] * _m.m[3][0];
	float A0223 = _m.m[2][0] * _m.m[3][2] - _m.m[2][2] * _m.m[3][0];
	float A0123 = _m.m[2][0] * _m.m[3][1] - _m.m[2][1] * _m.m[3][0];
	float A2313 = _m.m[1][2] * _m.m[3][3] - _m.m[1][3] * _m.m[3][2];
	float A1313 = _m.m[1][1] * _m.m[3][3] - _m.m[1][3] * _m.m[3][1];
	float A1213 = _m.m[1][1] * _m.m[3][2] - _m.m[1][2] * _m.m[3][1];
	float A2312 = _m.m[1][2] * _m.m[2][3] - _m.m[1][3] * _m.m[2][2];
	float A1312 = _m.m[1][1] * _m.m[2][3] - _m.m[1][3] * _m.m[2][1];
	float A1212 = _m.m[1][1] * _m.m[2][2] - _m.m[1][2] * _m.m[2][1];
	float A0313 = _m.m[1][0] * _m.m[3][3] - _m.m[1][3] * _m.m[3][0];
	float A0213 = _m.m[1][0] * _m.m[3][2] - _m.m[1][2] * _m.m[3][0];
	float A0312 = _m.m[1][0] * _m.m[2][3] - _m.m[1][3] * _m.m[2][0];
	float A0212 = _m.m[1][0] * _m.m[2][2] - _m.m[1][2] * _m.m[2][0];
	float A0113 = _m.m[1][0] * _m.m[3][1] - _m.m[1][1] * _m.m[3][0];
	float A0112 = _m.m[1][0] * _m.m[2][1] - _m.m[1][1] * _m.m[2][0];

	float det = _m.m[0][0] * ( _m.m[1][1] * A2323 - _m.m[1][2] * A1323 + _m.m[1][3] * A1223 ) 
	    - _m.m[0][1] * ( _m.m[1][0] * A2323 - _m.m[1][2] * A0323 + _m.m[1][3] * A0223 ) 
	    + _m.m[0][2] * ( _m.m[1][0] * A1323 - _m.m[1][1] * A0323 + _m.m[1][3] * A0123 ) 
	    - _m.m[0][3] * ( _m.m[1][0] * A1223 - _m.m[1][1] * A0223 + _m.m[1][2] * A0123 );
	det = 1.f / det;

	return Matrix4(
	   det *   ( _m.m[1][1] * A2323 - _m.m[1][2] * A1323 + _m.m[1][3] * A1223 ),
	   det * - ( _m.m[0][1] * A2323 - _m.m[0][2] * A1323 + _m.m[0][3] * A1223 ),
	   det *   ( _m.m[0][1] * A2313 - _m.m[0][2] * A1313 + _m.m[0][3] * A1213 ),
	   det * - ( _m.m[0][1] * A2312 - _m.m[0][2] * A1312 + _m.m[0][3] * A1212 ),
	   det * - ( _m.m[1][0] * A2323 - _m.m[1][2] * A0323 + _m.m[1][3] * A0223 ),
	   det *   ( _m.m[0][0] * A2323 - _m.m[0][2] * A0323 + _m.m[0][3] * A0223 ),
	   det * - ( _m.m[0][0] * A2313 - _m.m[0][2] * A0313 + _m.m[0][3] * A0213 ),
	   det *   ( _m.m[0][0] * A2312 - _m.m[0][2] * A0312 + _m.m[0][3] * A0212 ),
	   det *   ( _m.m[1][0] * A1323 - _m.m[1][1] * A0323 + _m.m[1][3] * A0123 ),
	   det * - ( _m.m[0][0] * A1323 - _m.m[0][1] * A0323 + _m.m[0][3] * A0123 ),
	   det *   ( _m.m[0][0] * A1313 - _m.m[0][1] * A0313 + _m.m[0][3] * A0113 ),
	   det * - ( _m.m[0][0] * A1312 - _m.m[0][1] * A0312 + _m.m[0][3] * A0112 ),
	   det * - ( _m.m[1][0] * A1223 - _m.m[1][1] * A0223 + _m.m[1][2] * A0123 ),
	   det *   ( _m.m[0][0] * A1223 - _m.m[0][1] * A0223 + _m.m[0][2] * A0123 ),
	   det * - ( _m.m[0][0] * A1213 - _m.m[0][1] * A0213 + _m.m[0][2] * A0113 ),
	   det *   ( _m.m[0][0] * A1212 - _m.m[0][1] * A0212 + _m.m[0][2] * A0112 )
	);
}
	
} // namespace math
} // namespace yae
