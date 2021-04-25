#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace yae {

struct Vector2 : public glm::vec2
{
	Vector2() {}
	Vector2(float _value) { x = _value; y = _value; }
	Vector2(float _x, float _y) { x = _x; y = _y; }

	static const Vector2 ZERO;
};


struct Vector3 : public glm::vec3
{
	Vector3() {}
	Vector3(float _value) { x = _value; y = _value; z = _value; }
	Vector3(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }

	static const Vector3 ZERO;
};

} // namespace yae
