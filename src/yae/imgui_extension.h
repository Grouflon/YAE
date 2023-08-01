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
YAE_API bool DragVector(const char* _label, yae::Vector2* _v, float _speed = 1.f, float _min = 0.f, float _max = 0.f);
YAE_API bool DragVector(const char* _label, yae::Vector3* _v, float _speed = 1.f, float _min = 0.f, float _max = 0.f);
YAE_API bool DragVector(const char* _label, yae::Vector4* _v, float _speed = 1.f, float _min = 0.f, float _max = 0.f);
YAE_API bool DragRotation(const char* _label, yae::Quaternion* _q, float _speed = 1.f);
YAE_API bool DragMatrix(const char* _label, yae::Matrix3* _m, float _speed = 1.f);
YAE_API bool DragMatrix(const char* _label, yae::Matrix4* _m, float _speed = 1.f);

YAE_API bool EditMirrorClassInstance(void* _instance, const mirror::Class* _class);
YAE_API bool EditMirrorType(const char* _name, void* _data, const mirror::TypeDesc* _type);

} // namespace ImGui
