#pragma once

#include <yae/types.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace yae {

struct YAELIB_API Vector2 : public glm::vec2
{
	Vector2() {}
	Vector2(float _value) { x = _value; y = _value; }
	Vector2(float _x, float _y) { x = _x; y = _y; }

	static const Vector2 ZERO;
	static const Vector2 ONE;
};


struct YAELIB_API Vector3 : public glm::vec3
{
	Vector3() {}
	Vector3(float _value) { x = _value; y = _value; z = _value; }
	Vector3(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }

	static const Vector3 ZERO;
	static const Vector3 ONE;
};

} // namespace yae
