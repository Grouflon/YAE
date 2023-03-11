#include <yae/math/matrix3.h>

namespace yae {
namespace matrix4 {

Matrix4 makeTranslation(const Vector3& _translation)
{
	return Matrix4(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		_translation.x, _translation.y, _translation.z, 1.f
	);
}

Matrix4 makeRotation(const Quaternion& _rotation)
{
	return makeFromMatrix3(matrix3::makeRotation(_rotation));
}

Matrix4 makeScale(const Vector3& _scale)
{
	return Matrix4(
		_scale.x, 0.f, 0.f, 0.f,
		0.f, _scale.y, 0.f, 0.f,
		0.f, 0.f, _scale.z, 0.f,
		0.f, 0.f, 0.f, 1.f
	);
}

Matrix4 makeTransform(const Vector3& _position, const Quaternion& _rotation, const Vector3& _scale)
{
	Matrix4 t = makeTranslation(_position);
	Matrix4 r = makeRotation(_rotation);
	Matrix4 s = makeScale(_scale);
	return t * r * s;
}

Matrix4 makePerspective(float _fov, float _aspectRatio, float _nearPlane, float _farPlane)
{
	return Matrix4();
}

Matrix4 makeFromMatrix3(const Matrix3& _m)
{
	return Matrix4(
		_m.m[0][0], _m.m[0][1], _m.m[0][2], 0.f,
		_m.m[1][0], _m.m[1][1], _m.m[1][2], 0.f,
		_m.m[2][0], _m.m[2][1], _m.m[2][2], 0.f,
		0.f, 0.f, 0.f, 1.f
	);
}

Matrix4 translate(const Matrix4& _m, const Vector3& _translation)
{
	return _m * makeTranslation(_translation);
}

Matrix4 rotate(const Matrix4& _m, const Quaternion& _rotation)
{
	return _m * makeRotation(_rotation);
}

Matrix4 scale(const Matrix4& _m, const Vector3& _scale)
{
	return _m * makeScale(_scale);
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
	
} // namespace matrix4
} // namespace yae
