#include <yae/math/vector2.h>
#include <yae/math/vector3.h>

#include <cmath>

namespace yae {

template <>
inline bool isEqual(const Quaternion& _a, const Quaternion& _b, float _threshold)
{
  return isEqual(_a.x, _b.x, _threshold) && isEqual(_a.y, _b.y, _threshold) && isEqual(_a.z, _b.z, _threshold) && isEqual(_a.w, _b.w, _threshold);
}

inline bool isIdentical(const Quaternion& _a, const Quaternion& _b, float _threshold)
{
  Quaternion normalizedA = quaternion::safeNormalize(_a);
  Quaternion normalizedB = quaternion::safeNormalize(_b);
  return 
    isEqual(quaternion::dot(normalizedA, normalizedB), 1.f, _threshold) ||
    isEqual(quaternion::dot(normalizedA, normalizedB * -1.f), 1.f, _threshold);
}

namespace quaternion {

Quaternion makeFromEuler(float _pitch, float _yaw, float _roll)
{
  Vector3 c = {
    cos(_pitch * .5f),
    cos(_yaw * .5f),
    cos(_roll * .5f)
  };
  Vector3 s = {
    sin(_pitch * .5f),
    sin(_yaw * .5f),
    sin(_roll * .5f)
  };

  Quaternion result;
  result.w = c.x * c.y * c.z + s.x * s.y * s.z;
  result.x = s.x * c.y * c.z - c.x * s.y * s.z;
  result.y = c.x * s.y * c.z + s.x * c.y * s.z;
  result.z = c.x * c.y * s.z - s.x * s.y * c.z;
  return result;
}

Quaternion makeFromEuler(const Vector3& _radianAngles)
{
  return makeFromEuler(_radianAngles.x, _radianAngles.y, _radianAngles.z);
}

Quaternion makeFromAngleAxis(float _radianAngle, const Vector3& _axis)
{
  return Quaternion(
    _axis.x * sin(_radianAngle/2.f),
    _axis.y * sin(_radianAngle/2.f),
    _axis.z * sin(_radianAngle/2.f),
    cos(_radianAngle/2.f)
  );
}

Quaternion makeFromMatrix3(const Matrix3& _m)
{
  // glm/gtc/quaternion.inl:81
  float fourXSquaredMinus1 = _m.m[0][0] - _m.m[1][1] - _m.m[2][2];
  float fourYSquaredMinus1 = _m.m[1][1] - _m.m[0][0] - _m.m[2][2];
  float fourZSquaredMinus1 = _m.m[2][2] - _m.m[0][0] - _m.m[1][1];
  float fourWSquaredMinus1 = _m.m[0][0] + _m.m[1][1] + _m.m[2][2];

  int biggestIndex = 0;
  float fourBiggestSquaredMinus1 = fourWSquaredMinus1;
  if(fourXSquaredMinus1 > fourBiggestSquaredMinus1)
  {
    fourBiggestSquaredMinus1 = fourXSquaredMinus1;
    biggestIndex = 1;
  }
  if(fourYSquaredMinus1 > fourBiggestSquaredMinus1)
  {
    fourBiggestSquaredMinus1 = fourYSquaredMinus1;
    biggestIndex = 2;
  }
  if(fourZSquaredMinus1 > fourBiggestSquaredMinus1)
  {
    fourBiggestSquaredMinus1 = fourZSquaredMinus1;
    biggestIndex = 3;
  }

  float biggestVal = sqrt(fourBiggestSquaredMinus1 + 1.f) * 0.5f;
  float mult = 0.25f / biggestVal;

  switch(biggestIndex)
  {
  case 0:
    return Quaternion(biggestVal, (_m.m[1][2] - _m.m[2][1]) * mult, (_m.m[2][0] - _m.m[0][2]) * mult, (_m.m[0][1] - _m.m[1][0]) * mult);
  case 1:
    return Quaternion((_m.m[1][2] - _m.m[2][1]) * mult, biggestVal, (_m.m[0][1] + _m.m[1][0]) * mult, (_m.m[2][0] + _m.m[0][2]) * mult);
  case 2:
    return Quaternion((_m.m[2][0] - _m.m[0][2]) * mult, (_m.m[0][1] + _m.m[1][0]) * mult, biggestVal, (_m.m[1][2] + _m.m[2][1]) * mult);
  case 3:
    return Quaternion((_m.m[0][1] - _m.m[1][0]) * mult, (_m.m[2][0] + _m.m[0][2]) * mult, (_m.m[1][2] + _m.m[2][1]) * mult, biggestVal);
  default: // Silence a -Wswitch-default warning in GCC. Should never actually get here. Assert is just for sanity.
    YAE_ASSERT(false);
    return quaternion::IDENTITY;
  }
}

float lengthSquared(const Quaternion& _q)
{
  return _q.x*_q.x + _q.y*_q.y + _q.z*_q.z + _q.w*_q.w;
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
  Vector3 start = vector3::normalize(_start);
  Vector3 dest = vector3::normalize(_dest);

  float cosTheta = vector3::dot(start, dest);
  Vector3 rotationAxis;

  if (cosTheta < -1.f + 0.001f){
    // special case when vectors in opposite directions:
    // there is no "ideal" rotation axis
    // So guess one; any will do as long as it's perpendicular to start
    rotationAxis = vector3::cross(Vector3(0.0f, 0.0f, 1.0f), start);
    if (vector3::lengthSquared(rotationAxis) < 0.01) // bad luck, they were parallel, try again!
      rotationAxis = vector3::cross(Vector3(1.0f, 0.0f, 0.0f), start);

    rotationAxis = vector3::normalize(rotationAxis);
    return quaternion::makeFromAngleAxis(180.0f * R2D, rotationAxis);
  }

  rotationAxis = vector3::cross(start, dest);

  float s = sqrt( (1.f+cosTheta)*2.f );
  float invs = 1.f / s;

  return Quaternion(
    s * 0.5f, 
    rotationAxis.x * invs,
    rotationAxis.y * invs,
    rotationAxis.z * invs
  );
}

float dot(const Quaternion& _a, const Quaternion& _b)
{
  return _a.x*_b.x + _a.y*_b.y + _a.z*_b.z + _a.w*_b.w;
}

Quaternion conjugate(const Quaternion& _q)
{
  return Quaternion(-_q.x, -_q.y, -_q.z, _q.w);
}

Quaternion inverse(const Quaternion& _q)
{
  return conjugate(_q) / dot(_q, _q);
}

Vector3 forward(const Quaternion& _q)
{
  return quaternion::rotate(_q, vector3::FORWARD);
}

Vector3 right(const Quaternion& _q)
{
  return quaternion::rotate(_q, vector3::RIGHT);
}

Vector3 up(const Quaternion& _q)
{
  return quaternion::rotate(_q, vector3::UP);
}

float roll(const Quaternion& _q)
{
  const float y = 2.f * (_q.x * _q.y + _q.w * _q.z);
  const float x = _q.w * _q.w + _q.x * _q.x - _q.y * _q.y - _q.z * _q.z;

  if(isZero(Vector2(x, y), VERY_SMALL_NUMBER)) //avoid atan2(0,0) - handle singularity - Matiis
    return 0.f;

  return std::atan2(y, x);
}

float pitch(const Quaternion& _q)
{
  const float y = 2.f * (_q.y * _q.z + _q.w * _q.x);
  const float x = _q.w * _q.w - _q.x * _q.x - _q.y * _q.y + _q.z * _q.z;

  if(isZero(Vector2(x, y), VERY_SMALL_NUMBER)) //avoid atan2(0,0) - handle singularity - Matiis
    return 2.f * std::atan2(_q.x, _q.w);

  return std::atan2(y, x);
}

float yaw(const Quaternion& _q)
{
  return std::asin(clamp(-2.f * (_q.x * _q.z - _q.w * _q.y), -1.f, 1.f));
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
    return 2.0f * vector3::dot(u, _v) * u
          + (s*s - vector3::dot(u, u)) * _v
          + 2.0f * s * vector3::cross(u, _v);
}

} // namespace quaternion
} // namespace yae