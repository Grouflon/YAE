#pragma once

#include <yae/types.h>

namespace ImGui
{

YAE_API bool DragVector3(const char* _label, yae::Vector3& _v, float _speed = 1.f, float _min = 0.f, float _max = 0.f);
YAE_API bool DragRotation(const char* _label, yae::Quaternion& _q);

} // namespace ImGui

namespace yae {

YAE_API void imgui_matrix4(const Matrix4& _matrix);
YAE_API void imgui_matrix4(const float _matrix[16]);
YAE_API void imgui_matrix3(const Matrix3& _matrix);
YAE_API void imgui_matrix3(const float _matrix[9]);
YAE_API void imgui_quaternion(const Quaternion& _q);

} // namespace yae
