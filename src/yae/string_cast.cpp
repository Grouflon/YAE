#include "string_cast.h"

#include <core/string.h>

namespace yae {

String toString(const Vector2& _v)
{
	return string::format("{%.2f, %.2f}", _v.x, _v.y);
}

String toString(const Vector3& _v)
{
	return string::format("{%.2f, %.2f, %.2f}", _v.x, _v.y, _v.z);
}

String toString(const Vector4& _v)
{
	return string::format("{%.2f, %.2f, %.2f, %.2f}", _v.x, _v.y, _v.z, _v.w);
}

String toString(const Quaternion& _q)
{
	return string::format("{%.2f, %.2f, %.2f, %.2f}", _q.w, _q.x, _q.y, _q.z);
}

} // namespace yae
