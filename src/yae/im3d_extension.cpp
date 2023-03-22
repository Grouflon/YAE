#include "im3d_extension.h"

#include <yae/math_3d.h>

#include <im3d/im3d.h>

namespace Im3d {

void DrawRotation(const yae::Quaternion& _q, float _radius)
{
	yae::Vector3 forward = yae::math::forward(_q) * _radius;
	yae::Vector3 up = yae::math::up(_q) * _radius;
	yae::Vector3 right = yae::math::right(_q) * _radius;

    Im3d::BeginLines();
    	Im3d::SetColor(Im3d::Color_Red);
    	Im3d::Vertex(0.f, 0.f, 0.f);
    	Im3d::Vertex(forward.x, forward.y, forward.z);
    Im3d::End();
    Im3d::BeginLines();
    	Im3d::SetColor(Im3d::Color_Green);
    	Im3d::Vertex(0.f, 0.f, 0.f);
    	Im3d::Vertex(up.x, up.y, up.z);
    Im3d::End();
    Im3d::BeginLines();
    	Im3d::SetColor(Im3d::Color_Blue);
    	Im3d::Vertex(0.f, 0.f, 0.f);
    	Im3d::Vertex(right.x, right.y, right.z);
    Im3d::End();
}

} // namespace Im3d
