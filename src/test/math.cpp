#include "math.h"

#include <yae/math_3d.h>
#include <yae/random.h>

#include <test/test_macros.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace yae;

namespace math {

glm::vec2 toGlm(const Vector2& _v) { return glm::vec2(_v.x, _v.y); }
glm::vec3 toGlm(const Vector3& _v) { return glm::vec3(_v.x, _v.y, _v.z); }
glm::vec4 toGlm(const Vector4& _v) { return glm::vec4(_v.x, _v.y, _v.z, _v.w); }
glm::quat toGlm(const Quaternion& _q) { return glm::quat(_q.w, _q.x, _q.y, _q.z); }

Vector2 toYae(const glm::vec2& _v) { return Vector2(_v.x, _v.y); }
Vector3 toYae(const glm::vec3& _v) { return Vector3(_v.x, _v.y, _v.z); }
Vector4 toYae(const glm::vec4& _v) { return Vector4(_v.x, _v.y, _v.z, _v.w); }
Quaternion toYae(const glm::quat& _q) { return Quaternion(_q.x, _q.y, _q.z, _q.w); }

void testVectors()
{
    {
        Vector2 a = { random::range(-100.f, 100.f), random::range(-100.f, 100.f) };
        Vector2 b = { random::range(-100.f, 100.f), random::range(-100.f, 100.f) };

        glm::vec2 g_a = toGlm(a);
        glm::vec2 g_b = toGlm(b);

        TEST(isZero(a - a));
        TEST(toGlm(a - b) == (g_a - g_b));
        TEST(toGlm(a + b) == (g_a + g_b));
        TEST(toGlm(a * b) == (g_a * g_b));
        TEST(toGlm(a / b) == (g_a / g_b));
        TEST(isEqual(vector2::dot(a, b), glm::dot(g_a, g_b)));
    }
    
    {
        Vector3 a = { random::range(-100.f, 100.f), random::range(-100.f, 100.f), random::range(-100.f, 100.f) };
        Vector3 b = { random::range(-100.f, 100.f), random::range(-100.f, 100.f), random::range(-100.f, 100.f) };

        glm::vec3 g_a = toGlm(a);
        glm::vec3 g_b = toGlm(b);

        TEST(isZero(a - a));
        TEST(toGlm(a - b) == (g_a - g_b));
        TEST(toGlm(a + b) == (g_a + g_b));
        TEST(toGlm(a * b) == (g_a * g_b));
        TEST(toGlm(a / b) == (g_a / g_b));
        TEST(isEqual(vector3::dot(a, b), glm::dot(g_a, g_b)));
    }

    {
        Vector4 a = { random::range(-100.f, 100.f), random::range(-100.f, 100.f), random::range(-100.f, 100.f), random::range(-100.f, 100.f) };
        Vector4 b = { random::range(-100.f, 100.f), random::range(-100.f, 100.f), random::range(-100.f, 100.f), random::range(-100.f, 100.f) };

        glm::vec4 g_a = toGlm(a);
        glm::vec4 g_b = toGlm(b);

        TEST(isZero(a - a));
        TEST(toGlm(a - b) == (g_a - g_b));
        TEST(toGlm(a + b) == (g_a + g_b));
        TEST(toGlm(a * b) == (g_a * g_b));
        TEST(toGlm(a / b) == (g_a / g_b));
        TEST(isEqual(vector4::dot(a, b), glm::dot(g_a, g_b), 0.001f));
    }
}


void testQuaternion()
{
    // Tests for very small angle values
    {
        Quaternion q = Quaternion(-0.41228247f, -0.57448214f, -0.41231012f, 0.57445085f);
        glm::quat g_q = toGlm(q);

        TEST(toGlm(q) == g_q);
        glm::vec3 g_a = glm::eulerAngles(g_q);
        Vector3 a = quaternion::euler(q);
        TEST(isEqual(a, toYae(g_a)));
    }

    // Noticed that some cases could fail depending on the input angle. Put that in a randomized loop to try to catch all errors
    for (u32 i = 0; i < 512; ++i)
    {
        Vector3 a = Vector3(D2R * random::range(-180.f, 180.f), D2R * random::range(-180.f, 180.f), D2R * random::range(-180.f, 180.f));
        Quaternion qa = quaternion::makeFromEuler(a);
        glm::quat g_qa = glm::quat(toGlm(a));

        // euler angles
        TEST(toGlm(qa) == g_qa);
        glm::vec3 g_ea = glm::eulerAngles(g_qa);
        Vector3 ea = quaternion::euler(qa);
        TEST(isEqual(ea, toYae(g_ea)));

        glm::quat g_qaa = glm::quat(g_ea);        
        Quaternion qaa = quaternion::makeFromEuler(ea);

        TEST(isIdentical(toYae(g_qa), toYae(g_qaa)));
        TEST(isIdentical(qa, qaa));

        TEST(isIdentical(qa, qaa));
    }
    
    Vector3 a = Vector3(D2R * random::range(-180.f, 180.f), D2R * random::range(-180.f, 180.f), D2R * random::range(-180.f, 180.f));
    Quaternion qa = quaternion::makeFromEuler(a);
    glm::quat g_qa = glm::quat(toGlm(a));

    // Inversion
    {
        Quaternion i = quaternion::inverse(qa);
        glm::quat g_i = glm::inverse(g_qa);
        TEST(isEqual(i, toYae(g_i)));

        Quaternion y = qa * i;
        glm::quat g = g_qa * g_i;
        TEST(isEqual(y, toYae(g)));
        TEST(isIdentical(y, quaternion::IDENTITY));
    }

    {
        Vector3 v1 = Vector3(
            random::range(-10.f, 10.f),
            random::range(-10.f, 10.f),
            random::range(-10.f, 10.f)
        );
        Quaternion q1 = quaternion::makeFromEuler(
            random::range(-180.f, 180.f) * D2R,
            random::range(-180.f, 180.f) * D2R,
            random::range(-180.f, 180.f) * D2R
        );
        Vector3 v2 = quaternion::rotate(q1, v1);
        Quaternion q2 = quaternion::rotationBetween(v1, v2);
        glm::quat g_q2 = glm::rotation(toGlm(vector3::safeNormalize(v1)), toGlm(vector3::safeNormalize(v2)));

        Vector3 up = quaternion::up(q2);
        Vector3 forward = quaternion::forward(q2);
        Vector3 right = quaternion::right(q2);

        TEST(isEqual(vector3::length(v1), vector3::length(v2)));
        TEST(toGlm(q2) == g_q2);
        TEST(isIdentical(q1, q2));   
        TEST(isEqual(quaternion::dot(q1, q2), glm::dot(toGlm(q1), toGlm(q2))));
    }
    

    /*
    glm::vec3 gva = {1.12f, -3.54f, 0.23f};
    glm::vec3 gvaa = (*((glm::quat*)&qa)) * glm::vec3(1.12f, -3.54f, 0.23f);

    Vector3 b = {D2R * 45.f, 0.f, 0.f};
    Quaternion qb = quaternion::makeFromEuler(b);
    TEST(isEqual(quaternion::euler(qb * qb), {D2R * 90.f, 0.f, 0.f}));
    TEST(isEqual(quaternion::rotate(qb, {1.f, 0.f, 0.f}), {1.f, 0.f, 0.f}));
    TEST(isEqual(quaternion::rotate(qb, {0.f, 1.f, 0.f}), vector3::normalize({0.f, 1.f, 1.f})));
    TEST(isEqual(quaternion::rotate(quaternion::inverse(qb), {0.f, 1.f, 0.f}), vector3::normalize({0.f, 1.f, -1.f})));

    Vector3 f = quaternion::forward(quaternion::IDENTITY);
    Vector3 r = quaternion::right(quaternion::IDENTITY);
    Vector3 u = quaternion::up(quaternion::IDENTITY);
    TEST(isEqual(f, {1.f, 0.f, 0.f}));
    TEST(isEqual(r, {0.f, 0.f, 1.f}));
    TEST(isEqual(u, {0.f, 1.f, 0.f}));
    */
}

} // namespace math
