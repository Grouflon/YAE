#include "imgui_extension.h"

#include <yae/math_3d.h>
#include <yae/containers/Array.h>

#include <imgui/imgui.h>
#include <mirror/mirror.h>

#include <limits>

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

bool DragVector(const char* _label, yae::Vector2* _v, float _speed, float _min, float _max)
{
	return DragFloat2(_label, (float*)_v, _speed, _min, _max);
}

bool DragVector(const char* _label, yae::Vector3* _v, float _speed, float _min, float _max)
{
	return DragFloat3(_label, (float*)_v, _speed, _min, _max);
}

bool DragVector(const char* _label, yae::Vector4* _v, float _speed, float _min, float _max)
{
	return DragFloat4(_label, (float*)_v, _speed, _min, _max);
}

bool DragRotation(const char* _label, yae::Quaternion* _q, float _speed)
{
	YAE_ASSERT(_q != nullptr);
	yae::Vector3 euler = yae::math::euler(*_q) * yae::R2D;
	bool result = DragVector(_label, &euler, _speed);
	*_q = yae::Quaternion::FromEuler(euler * yae::D2R);
	return result;
}

bool DragMatrix(const char* _label, yae::Matrix3* _m, float _speed)
{
	bool result = false;
	ImGui::PushID(yae::math::data((*_m)[0]));
	result = ImGui::DragVector(_label, &(*_m)[0], _speed) || result;
	ImGui::PopID();
	ImGui::PushID(yae::math::data((*_m)[1]));
	result = ImGui::DragVector("", &(*_m)[1], _speed) || result;
	ImGui::PopID();
	ImGui::PushID(yae::math::data((*_m)[2]));
	result = ImGui::DragVector("", &(*_m)[2], _speed) || result;
	ImGui::PopID();
	return result;
}

bool DragMatrix(const char* _label, yae::Matrix4* _m, float _speed)
{
	bool result = false;
	ImGui::PushID(yae::math::data((*_m)[0]));
	result = ImGui::DragVector(_label, &(*_m)[0], _speed) || result;
	ImGui::PopID();
	ImGui::PushID(yae::math::data((*_m)[1]));
	result = ImGui::DragVector("", &(*_m)[1], _speed) || result;
	ImGui::PopID();
	ImGui::PushID(yae::math::data((*_m)[2]));
	result = ImGui::DragVector("", &(*_m)[2], _speed) || result;
	ImGui::PopID();
	ImGui::PushID(yae::math::data((*_m)[3]));
	result = ImGui::DragVector("", &(*_m)[3], _speed) || result;
	ImGui::PopID();
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
	const float v_speed = 0.2f;

	if (_type == mirror::GetTypeDesc<yae::String>())
	{
		yae::String* stringPtr = (yae::String*)_data;
		return ImGui::InputText(_name, stringPtr);
	}
	else if (_type == mirror::GetTypeDesc<yae::Vector2>())
	{
		yae::Vector2* vectorPtr = (yae::Vector2*)_data;
		return ImGui::DragVector(_name, vectorPtr, v_speed);
	}
	else if (_type == mirror::GetTypeDesc<yae::Vector3>())
	{
		yae::Vector3* vectorPtr = (yae::Vector3*)_data;
		return ImGui::DragVector(_name, vectorPtr, v_speed);
	}
	else if (_type == mirror::GetTypeDesc<yae::Vector4>())
	{
		yae::Vector4* vectorPtr = (yae::Vector4*)_data;
		return ImGui::DragVector(_name, vectorPtr, v_speed);
	}
	else if (_type == mirror::GetTypeDesc<yae::Quaternion>())
	{
		yae::Quaternion* quatPtr = (yae::Quaternion*)_data;
		return ImGui::DragRotation(_name, quatPtr, v_speed);
	}
	else if (_type == mirror::GetTypeDesc<yae::Matrix3>())
	{
		yae::Matrix3* matrixPtr = (yae::Matrix3*)_data;
		return ImGui::DragMatrix(_name, matrixPtr, v_speed);
	}
	else if (_type == mirror::GetTypeDesc<yae::Matrix4>())
	{
		yae::Matrix4* matrixPtr = (yae::Matrix4*)_data;
		return ImGui::DragMatrix(_name, matrixPtr, v_speed);
	}

	switch(_type->getType())
	{
	case mirror::Type_bool:
		{
			return ImGui::Checkbox(_name, (bool*)_data);
		}
		break;

	case mirror::Type_char:
		{
			char* charPtr = (char*)_data;
			char buf[2] = {*charPtr, 0};
			bool result = ImGui::InputText(_name, buf, 2);
			*charPtr = buf[0];
			return result;
		}
		break;

	case mirror::Type_int8:
		{
			i8* dataPtr = (i8*)_data;
			int value = *dataPtr;
			bool result = DragInt(_name, &value, v_speed, std::numeric_limits<i8>::lowest(), std::numeric_limits<i8>::max());
			*dataPtr = value;
			return result;
		}
		break;

	case mirror::Type_int16:
		{
			i16* dataPtr = (i16*)_data;
			int value = *dataPtr;
			bool result = DragInt(_name, &value, v_speed, std::numeric_limits<i16>::lowest(), std::numeric_limits<i16>::max());
			*dataPtr = value;
			return result;
		}
		break;

	case mirror::Type_int32:
		{
			i32* dataPtr = (i32*)_data;
			int value = *dataPtr;
			bool result = DragInt(_name, &value, v_speed, std::numeric_limits<i32>::lowest(), std::numeric_limits<i32>::max());
			*dataPtr = value;
			return result;
		}
		break;

	case mirror::Type_int64:
		{
			i64* dataPtr = (i64*)_data;
			int value = *dataPtr;
			bool result = DragInt(_name, &value, v_speed, std::numeric_limits<i32>::lowest(), std::numeric_limits<i32>::max());
			*dataPtr = value;
			return result;
		}
		break;

	case mirror::Type_uint8:
		{
			u8* dataPtr = (u8*)_data;
			int value = *dataPtr;
			bool result = DragInt(_name, &value, v_speed, std::numeric_limits<u8>::lowest(), std::numeric_limits<u8>::max());
			*dataPtr = value;
			return result;
		}
		break;

	case mirror::Type_uint16:
		{
			u16* dataPtr = (u16*)_data;
			int value = *dataPtr;
			bool result = DragInt(_name, &value, v_speed, std::numeric_limits<u16>::lowest(), std::numeric_limits<u16>::max());
			*dataPtr = value;
			return result;
		}
		break;

	case mirror::Type_uint32:
		{
			u32* dataPtr = (u32*)_data;
			int value = *dataPtr;
			bool result = DragInt(_name, &value, v_speed, std::numeric_limits<u32>::lowest(), std::numeric_limits<i32>::max());
			*dataPtr = value;
			return result;
		}
		break;

	case mirror::Type_uint64:
		{
			u64* dataPtr = (u64*)_data;
			int value = *dataPtr;
			bool result = DragInt(_name, &value, v_speed, std::numeric_limits<u64>::lowest(), std::numeric_limits<i32>::max());
			*dataPtr = value;
			return result;
		}
		break;

	case mirror::Type_float:
		{
			float* dataPtr = (float*)_data;
			float value = *dataPtr;
			bool result = DragFloat(_name, &value, v_speed, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			*dataPtr = value;
			return result;
		}
		break;

	case mirror::Type_double:
		{
			double* dataPtr = (double*)_data;
			float value = *dataPtr;
			bool result = DragFloat(_name, &value, v_speed, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			*dataPtr = value;
			return result;
		}
		break;

	case mirror::Type_Enum:
		{
			const mirror::Enum* enumType = _type->asEnum();
			const mirror::TypeDesc* subType = enumType->getSubType();
			if (subType == nullptr)
			{
				subType = mirror::GetTypeDesc<int>();
			}
			YAE_ASSERT(subType != nullptr);

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

	case mirror::Type_FixedSizeArray:
		{
			const mirror::FixedSizeArray* arrayType = _type->asFixedSizeArray();
			const mirror::TypeDesc* subType = arrayType->getSubType();
			const int elementCount = (int)arrayType->getElementCount();
			YAE_ASSERT(subType != nullptr);

			bool result = false;
			if (ImGui::TreeNodeEx(_data, 0, "%s(%d)", _name, elementCount))
			{
				int flags = 0;
				if (ImGui::BeginTable(_name, 2, flags))
		        {
		            ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed, 25.0f);
		            ImGui::TableSetupColumn("Data", ImGuiTableColumnFlags_WidthStretch);

		            for (int row = 0; row < elementCount; row++)
            		{
		                ImGui::TableNextRow();
		                ImGui::TableSetColumnIndex(0);
		                ImGui::Text("[%d]", row);
		                ImGui::TableSetColumnIndex(1);
		                void* data = arrayType->getDataAt(_data, row);
		                ImGui::PushID(data);
		                result = EditMirrorType("", data, subType) || result;
		                ImGui::PopID();
		            }
		            ImGui::EndTable();
		        }

				ImGui::TreePop();
			}
			return result;
		}
		break;

	case mirror::Type_Class:
		{
			if (_type->asClass()->getMembersCount() != 0)
			{
				bool result = false;
				if (ImGui::TreeNodeEx(_name, ImGuiTreeNodeFlags_DefaultOpen))
				{
					result = EditMirrorClassInstance(_data, _type->asClass());
					ImGui::TreePop();
				}
				return result;
			}
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
		if (member->getMetaDataSet().findMetaData("Hidden"))
			continue;

		result = ImGui::EditMirrorType(member->getName(), member->getInstanceMemberPointer(_instance), member->getType()) || result;
	}
	return result;
}

} // namespace ImGui
