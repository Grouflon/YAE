#pragma once

#include <yae/types.h>

#include <imgui/imgui.h>

namespace mirror
{
	class Class;
	class TypeDesc;
}

namespace ImGui
{

YAE_API bool InputText(const char* _label, yae::String* _str, ImGuiInputTextFlags _flags = 0, ImGuiInputTextCallback _callback = NULL, void* _user_data = NULL);
YAE_API bool DragVector3(const char* _label, yae::Vector3& _v, float _speed = 1.f, float _min = 0.f, float _max = 0.f);
YAE_API bool DragRotation(const char* _label, yae::Quaternion& _q);

YAE_API bool EditMirrorClassInstance(void* _instance, const mirror::Class* _class);
YAE_API bool EditMirrorType(const char* _name, void* _data, const mirror::TypeDesc* _type);

} // namespace ImGui

namespace yae {

YAE_API void imgui_matrix4(const Matrix4& _matrix);
YAE_API void imgui_matrix4(const float _matrix[16]);
YAE_API void imgui_matrix3(const Matrix3& _matrix);
YAE_API void imgui_matrix3(const float _matrix[9]);
YAE_API void imgui_quaternion(const Quaternion& _q);

} // namespace yae
