#include <yae/math/vector2.h>
#include <yae/math/vector3.h>
#include <yae/math/glm_conversion.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <cmath>

namespace yae {

String toString(const Quaternion& _q)
{
	return string::format("{%.2f, %.2f, %.2f, %.2f}", _q.w, _q.x, _q.y, _q.z);
}

namespace math {

template <>
bool isEqual(const Quaternion& _a, const Quaternion& _b, float _threshold)
{
	return isEqual(_a.w, _b.w, _threshold) && isEqual(_a.x, _b.x, _threshold) && isEqual(_a.y, _b.y, _threshold) && isEqual(_a.z, _b.z, _threshold);
}

bool isIdentical(const Quaternion& _a, const Quaternion& _b, float _threshold)
{
	Quaternion normalizedA = safeNormalize(_a);
	Quaternion normalizedB = safeNormalize(_b);
	return 
		isEqual(dot(normalizedA, normalizedB), 1.f, _threshold) ||
		isEqual(dot(normalizedA, normalizedB * -1.f), 1.f, _threshold);
}

const float* data(const Quaternion& _q)
{
	return (const float*)&_q;
}

float* data(Quaternion& _q)
{
	return (float*)&_q;
}

float lengthSquared(const Quaternion& _q)
{
	return  _q.w*_q.w + _q.x*_q.x + _q.y*_q.y + _q.z*_q.z;
}

float length(const Quaternion& _q)
{
	return sqrt(lengthSquared(_q));
}

Quaternion normalize(const Quaternion& _q)
{
	float l = length(_q);
	YAE_ASSERT(l > SMALL_NUMBER);
	return _q / l;
}

Quaternion safeNormalize(const Quaternion& _q, float _threshold)
{
	float l = length(_q);
	if (l < _threshold)
		return Quaternion(0.f);
	return _q / l;
}

Quaternion rotationBetween(const Vector3& _start, const Vector3& _dest)
{
	Vector3 start = normalize(_start);
	Vector3 dest = normalize(_dest);

	return toYae(glm::rotation(toGlm(start), toGlm(dest)));
	/*
	float cosTheta = dot(start, dest);
	Vector3 rotationAxis;

	if (cosTheta >= 1.f - VERY_SMALL_NUMBER)
	{
		return Quaternion::IDENTITY;
	}

	if (cosTheta < -1.f + VERY_SMALL_NUMBER){
		// special case when vectors in opposite directions:
		// there is no "ideal" rotation axis
		// So guess one; any will do as long as it's perpendicular to start
		rotationAxis = cross(Vector3(0.0f, 0.0f, 1.0f), start);
		if (lengthSquared(rotationAxis) < VERY_SMALL_NUMBER) // bad luck, they were parallel, try again!
			rotationAxis = cross(Vector3(1.0f, 0.0f, 0.0f), start);

		rotationAxis = normalize(rotationAxis);
		return Quaternion::FromAngleAxis(PI, rotationAxis);
	}

	rotationAxis = cross(start, dest);

	float s = sqrt( (1.f+cosTheta)*2.f );
	float invs = 1.f / s;

	return Quaternion(
		s * 0.5f,
		rotationAxis.x * invs,
		rotationAxis.y * invs,
		rotationAxis.z * invs
	);
	*/
}

float dot(const Quaternion& _a, const Quaternion& _b)
{
	return _a.w*_b.w + _a.x*_b.x + _a.y*_b.y + _a.z*_b.z;
}

Quaternion conjugate(const Quaternion& _q)
{
	return toYae(glm::conjugate(toGlm(_q)));
}

Quaternion inverse(const Quaternion& _q)
{
	return toYae(glm::inverse(toGlm(_q)));
}

Vector3 forward(const Quaternion& _q)
{
	return rotate(_q, Vector3::FORWARD);
}

Vector3 right(const Quaternion& _q)
{
	return rotate(_q, Vector3::RIGHT);
}

Vector3 up(const Quaternion& _q)
{
	return rotate(_q, Vector3::UP);
}

float roll(const Quaternion& _q)
{
	return glm::roll(toGlm(_q));
	/*
	const float y = 2.f * (_q.x * _q.y + _q.w * _q.z);
	const float x = _q.w * _q.w + _q.x * _q.x - _q.y * _q.y - _q.z * _q.z;

	if(isZero(Vector2(x, y), VERY_SMALL_NUMBER)) //avoid atan2(0,0) - handle singularity - Matiis
		return 0.f;

	return std::atan2(y, x);
	*/
}

float pitch(const Quaternion& _q)
{
	return glm::pitch(toGlm(_q));
	/*
	const float y = 2.f * (_q.y * _q.z + _q.w * _q.x);
	const float x = _q.w * _q.w - _q.x * _q.x - _q.y * _q.y + _q.z * _q.z;

	if(isZero(Vector2(x, y), VERY_SMALL_NUMBER)) //avoid atan2(0,0) - handle singularity - Matiis
		return 2.f * std::atan2(_q.x, _q.w);

	return std::atan2(y, x);
	*/
}

float yaw(const Quaternion& _q)
{
	return glm::yaw(toGlm(_q));
	/*
	return std::asin(clamp(-2.f * (_q.x * _q.z - _q.w * _q.y), -1.f, 1.f));
	*/
}

Vector3 euler(const Quaternion& _q)
{
	return Vector3(pitch(_q), yaw(_q), roll(_q));
}

Vector3 rotate(const Quaternion& _q, const Vector3& _v)
{
	// algorithm found here:
	// https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
	// Extract the vector part of the quaternion
		Vector3 u(_q.x, _q.y, _q.z);

		// Extract the scalar part of the quaternion
		float s = _q.w;

		// Do the math
		return 2.0f * dot(u, _v) * u
					+ (s*s - dot(u, u)) * _v
					+ 2.0f * s * cross(u, _v);
}

} // namespace math
} // namespace yae