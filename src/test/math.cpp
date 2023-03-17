#include "math.h"

#include <yae/math_3d.h>
#include <yae/random.h>
#include <yae/math/glm_conversion.h>

#include <test/test_macros.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace yae;

namespace math {

void testVectors()
{
    {
        Vector2 a = { random::range(-100.f, 100.f), random::range(-100.f, 100.f) };
        Vector2 b = { random::range(-100.f, 100.f), random::range(-100.f, 100.f) };

        glm::vec2 g_a = toGlm(a);
        glm::vec2 g_b = toGlm(b);

        TEST(yae::math::isZero(a - a));
        TEST(toGlm(a - b) == (g_a - g_b));
        TEST(toGlm(a + b) == (g_a + g_b));
        TEST(toGlm(a * b) == (g_a * g_b));
        TEST(toGlm(a / b) == (g_a / g_b));
        TEST(yae::math::isEqual(yae::math::dot(a, b), glm::dot(g_a, g_b)));
    }
    
    {
        Vector3 a = { random::range(-100.f, 100.f), random::range(-100.f, 100.f), random::range(-100.f, 100.f) };
        Vector3 b = { random::range(-100.f, 100.f), random::range(-100.f, 100.f), random::range(-100.f, 100.f) };

        glm::vec3 g_a = toGlm(a);
        glm::vec3 g_b = toGlm(b);

        TEST(yae::math::isZero(a - a));
        TEST(toGlm(a - b) == (g_a - g_b));
        TEST(toGlm(a + b) == (g_a + g_b));
        TEST(toGlm(a * b) == (g_a * g_b));
        TEST(toGlm(a / b) == (g_a / g_b));
        TEST(yae::math::isEqual(yae::math::dot(a, b), glm::dot(g_a, g_b)));
    }

    {
        Vector4 a = { random::range(-100.f, 100.f), random::range(-100.f, 100.f), random::range(-100.f, 100.f), random::range(-100.f, 100.f) };
        Vector4 b = { random::range(-100.f, 100.f), random::range(-100.f, 100.f), random::range(-100.f, 100.f), random::range(-100.f, 100.f) };

        glm::vec4 g_a = toGlm(a);
        glm::vec4 g_b = toGlm(b);

        TEST(yae::math::isZero(a - a));
        TEST(toGlm(a - b) == (g_a - g_b));
        TEST(toGlm(a + b) == (g_a + g_b));
        TEST(toGlm(a * b) == (g_a * g_b));
        TEST(toGlm(a / b) == (g_a / g_b));
        TEST(yae::math::isEqual(yae::math::dot(a, b), glm::dot(g_a, g_b), 0.001f));
    }
}


void testQuaternion()
{
    // Test for named axes
    {
        Vector3 forward = yae::math::forward(Quaternion::IDENTITY);
        Vector3 up = yae::math::up(Quaternion::IDENTITY);
        Vector3 right = yae::math::right(Quaternion::IDENTITY);
        TEST(yae::math::isEqual(forward, {0.f, 0.f, 1.f}));
        TEST(yae::math::isEqual(up, {0.f, 1.f, 0.f}));
        TEST(yae::math::isEqual(right, {1.f, 0.f, 0.f}));
    }

    // Tests for very small angle values
    {
        Quaternion q = Quaternion(0.57445085f, -0.41228247f, -0.57448214f, -0.41231012f);
        glm::quat g_q = toGlm(q);

        TEST(toGlm(q) == g_q);
        glm::vec3 g_a = glm::eulerAngles(g_q);
        Vector3 a = yae::math::euler(q);
        TEST(yae::math::isEqual(a, toYae(g_a)));
    }

    // Noticed that some cases could fail depending on the input angle. Put that in a randomized loop to try to catch all errors
    for (u32 i = 0; i < 512; ++i)
    {
        Vector3 a = Vector3(D2R * random::range(-180.f, 180.f), D2R * random::range(-180.f, 180.f), D2R * random::range(-180.f, 180.f));
        Quaternion qa = Quaternion::FromEuler(a);
        glm::quat g_qa = glm::quat(toGlm(a));

        // euler angles
        TEST(yae::math::isEqual(qa, toYae(g_qa)));
        glm::vec3 g_ea = glm::eulerAngles(g_qa);
        Vector3 ea = yae::math::euler(qa);
        TEST(yae::math::isEqual(ea, toYae(g_ea)));

        glm::quat g_qaa = glm::quat(g_ea);        
        Quaternion qaa = Quaternion::FromEuler(ea);

        TEST(yae::math::isIdentical(toYae(g_qa), toYae(g_qaa)));
        TEST(yae::math::isIdentical(qa, qaa));

        TEST(yae::math::isIdentical(qa, qaa));
    }
    
    Vector3 a = Vector3(D2R * random::range(-180.f, 180.f), D2R * random::range(-180.f, 180.f), D2R * random::range(-180.f, 180.f));
    Quaternion qa = Quaternion::FromEuler(a);
    glm::quat g_qa = glm::quat(toGlm(a));

    // Inversion
    {
        Quaternion i = yae::math::inverse(qa);
        glm::quat g_i = glm::inverse(g_qa);
        TEST(yae::math::isEqual(i, toYae(g_i)));

        Quaternion y = qa * i;
        glm::quat g = g_qa * g_i;
        TEST(yae::math::isEqual(y, toYae(g)));
        TEST(yae::math::isIdentical(y, Quaternion::IDENTITY));
    }

    {
        Vector3 v1 = Vector3(
            random::range(-10.f, 10.f),
            random::range(-10.f, 10.f),
            random::range(-10.f, 10.f)
        );
        Quaternion q1 = Quaternion::FromEuler(
            random::range(-180.f, 180.f) * D2R,
            random::range(-180.f, 180.f) * D2R,
            random::range(-180.f, 180.f) * D2R
        );
        Vector3 v2 = yae::math::rotate(q1, v1);
        TEST(yae::math::isEqual(yae::math::length(v1), yae::math::length(v2)));

        v1 = yae::math::safeNormalize(v1);
        v2 = yae::math::safeNormalize(v2);
        Quaternion q2 = yae::math::rotationBetween(v1, v2);
        glm::quat g_q1 = toGlm(q1);
        glm::quat g_q2 = glm::rotation(toGlm(v1), toGlm(v2));
        TEST(yae::math::isEqual(q2, toYae(g_q2)));
        TEST(yae::math::isEqual(yae::math::dot(q1, q2), glm::dot(g_q1, g_q2)));

        Vector3 up = yae::math::up(q2);
        Vector3 forward = yae::math::forward(q2);
        Vector3 right = yae::math::right(q2);
    }
    

    /*
    glm::vec3 gva = {1.12f, -3.54f, 0.23f};
    glm::vec3 gvaa = (*((glm::quat*)&qa)) * glm::vec3(1.12f, -3.54f, 0.23f);

    Vector3 b = {D2R * 45.f, 0.f, 0.f};
    Quaternion qb = quaternion::makeFromEuler(b);
    TEST(yae::math::isEqual(quaternion::euler(qb * qb), {D2R * 90.f, 0.f, 0.f}));
    TEST(yae::math::isEqual(quaternion::rotate(qb, {1.f, 0.f, 0.f}), {1.f, 0.f, 0.f}));
    TEST(yae::math::isEqual(quaternion::rotate(qb, {0.f, 1.f, 0.f}), normalize({0.f, 1.f, 1.f})));
    TEST(yae::math::isEqual(quaternion::rotate(quaternion::inverse(qb), {0.f, 1.f, 0.f}), vector3::normalize({0.f, 1.f, -1.f})));

    Vector3 f = quaternion::forward(quaternion::IDENTITY);
    Vector3 r = quaternion::right(quaternion::IDENTITY);
    Vector3 u = quaternion::up(quaternion::IDENTITY);
    TEST(yae::math::isEqual(f, {1.f, 0.f, 0.f}));
    TEST(yae::math::isEqual(r, {0.f, 0.f, 1.f}));
    TEST(yae::math::isEqual(u, {0.f, 1.f, 0.f}));
    */
}

} // namespace math
