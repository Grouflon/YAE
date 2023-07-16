#include "imgui_extension.h"

#include <yae/math_3d.h>
#include <yae/containers/Array.h>

#include <imgui/imgui.h>
#include <mirror/mirror.h>

struct InputTextCallback_UserData
{
    yae::String*            Str;
    ImGuiInputTextCallback  ChainCallback;
    void*                   ChainCallbackUserData;
};

static int InputTextCallback(ImGuiInputTextCallbackData* _data)
{
    InputTextCallback_UserData* user_data = (InputTextCallback_UserData*)_data->UserData;
    if (_data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        // Resize string callback
        // If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
        yae::String* str = user_data->Str;
        IM_ASSERT(_data->Buf == str->c_str());
        str->resize(_data->BufTextLen);
        _data->Buf = (char*)str->c_str();
    }
    else if (user_data->ChainCallback)
    {
        // Forward to user callback, if any
        _data->UserData = user_data->ChainCallbackUserData;
        return user_data->ChainCallback(_data);
    }
    return 0;
}

namespace ImGui {

bool InputText(const char* _label, yae::String* _str, ImGuiInputTextFlags _flags, ImGuiInputTextCallback _callback, void* _user_data)
{
    IM_ASSERT((_flags & ImGuiInputTextFlags_CallbackResize) == 0);
    _flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData cb_user_data;
    cb_user_data.Str = _str;
    cb_user_data.ChainCallback = _callback;
    cb_user_data.ChainCallbackUserData = _user_data;
    return InputText(_label, (char*)_str->c_str(), _str->capacity() + 1, _flags, InputTextCallback, &cb_user_data);
}

bool DragVector3(const char* _label, yae::Vector3& _v, float _speed, float _min, float _max)
{
	return DragFloat3(_label, yae::math::data(_v), _speed, _min, _max);
}

bool DragRotation(const char* _label, yae::Quaternion& _q)
{
	yae::Vector3 euler = yae::math::euler(_q) * yae::R2D;
	bool result = DragVector3(_label, euler);
	_q = yae::Quaternion::FromEuler(euler * yae::D2R);
	return result;
}

template <typename T>
bool imgui_mirrorEnumCombo(const char* _name, void* _data, const mirror::Enum* _enum)
{
	T* valuePtr = (T*) _data;
	const char* currentValueString = "Unknown value";
	YAE_VERIFY(_enum->getStringFromValue(*valuePtr, currentValueString));

	bool modified = false;
	if (ImGui::BeginCombo(_name, currentValueString))
	{
		for (const mirror::EnumValue* value : _enum->getValues())
		{
			if (ImGui::Selectable(value->getName(), value->getValue() == (i64)*valuePtr))
			{
				*valuePtr = value->getValue();
				modified = true;
			}
		}
		ImGui::EndCombo();
	}
	return modified;
}


bool EditMirrorType(const char* _name, void* _data, const mirror::TypeDesc* _type)
{
	if (_type == mirror::GetTypeDesc<yae::String>())
	{
		yae::String* stringPtr = (yae::String*)_data;
		return ImGui::InputText(_name, stringPtr);
	}
	else if (_type == mirror::GetTypeDesc<yae::Vector3>())
	{
		yae::Vector3* vectorPtr = (yae::Vector3*)_data;
		return ImGui::DragVector3(_name, *vectorPtr);
	}
	else if (_type == mirror::GetTypeDesc<yae::Quaternion>())
	{
		yae::Quaternion* quatPtr = (yae::Quaternion*)_data;
		return ImGui::DragRotation(_name, *quatPtr);
	}

	switch(_type->getType())
	{
	case mirror::Type_Enum:
		{
			const mirror::Enum* enumType = _type->asEnum();
			const mirror::TypeDesc* subType = enumType->getSubType();

			switch(subType->getType())
			{
			case mirror::Type_int8: return imgui_mirrorEnumCombo<i8>(_name, _data, enumType);
			case mirror::Type_int16: return imgui_mirrorEnumCombo<i16>(_name, _data, enumType);
			case mirror::Type_int32: return imgui_mirrorEnumCombo<i32>(_name, _data, enumType);
			case mirror::Type_int64: return imgui_mirrorEnumCombo<i64>(_name, _data, enumType);
			case mirror::Type_uint8: return imgui_mirrorEnumCombo<u8>(_name, _data, enumType);
			case mirror::Type_uint16: return imgui_mirrorEnumCombo<u16>(_name, _data, enumType);
			case mirror::Type_uint32: return imgui_mirrorEnumCombo<u32>(_name, _data, enumType);
			case mirror::Type_uint64: return imgui_mirrorEnumCombo<u64>(_name, _data, enumType);
			default: YAE_ASSERT("Not an enum type"); return false;
			}
		}
		break;

	case mirror::Type_Class:
		{
			bool result = false;
			if (ImGui::TreeNodeEx(_name, ImGuiTreeNodeFlags_DefaultOpen))
			{
				result = EditMirrorClassInstance(_data, _type->asClass());
				ImGui::TreePop();
			}
			return result;
		}
		break;

	default:
		{
			ImGui::Text("%s: Unsupported type %s", _name, mirror::TypeToString(_type->getType()));
		}
		break;
	}
	return false;
}

bool EditMirrorClassInstance(void* _instance, const mirror::Class* _class)
{
	size_t membersCount = _class->getMembersCount();
	yae::DataArray<mirror::ClassMember*> members(&yae::scratchAllocator());
	members.resize(membersCount);
	_class->getMembers(members.data(), membersCount);

	bool result = false;
	for (mirror::ClassMember* member : members)
	{
		result = result || ImGui::EditMirrorType(member->getName(), member->getInstanceMemberPointer(_instance), member->getType());
	}
	return result;
}


} // namespace ImGui

namespace yae {

void imgui_matrix4(const Matrix4& _matrix)
{
	imgui_matrix4(math::data(_matrix));
}

void imgui_matrix4(const float _matrix[16])
{
	ImGui::Text("%.4f %.4f %.4f %.4f\n%.4f %.4f %.4f %.4f\n%.4f %.4f %.4f %.4f\n%.4f %.4f %.4f %.4f",
	_matrix[0], _matrix[1], _matrix[2], _matrix[3],
	_matrix[4], _matrix[5], _matrix[6], _matrix[7],
	_matrix[8], _matrix[9], _matrix[10], _matrix[11],
	_matrix[12], _matrix[13], _matrix[14], _matrix[15]
	);
}

void imgui_matrix3(const Matrix3& _matrix)
{
	imgui_matrix3(math::data(_matrix));
}

void imgui_matrix3(const float _matrix[9])
{
	ImGui::Text("%.4f %.4f %.4f\n%.4f %.4f %.4f\n%.4f %.4f %.4f",
	_matrix[0], _matrix[1], _matrix[2],
	_matrix[3], _matrix[4], _matrix[5],
	_matrix[6], _matrix[7], _matrix[8]
	);
}

void imgui_quaternion(const Quaternion& _q)
{
	ImGui::Text("%.4f %.4f %.4f %.4f",
		_q.x, _q.y, _q.z, _q.w
	);
}

} // namespace yae
