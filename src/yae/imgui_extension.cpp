#include "imgui_extension.h"

#include <yae/math_3d.h>
#include <core/containers/Array.h>
#include <yae/resources/Resource.h>
#include <yae/ResourceManager.h>
#include <core/filesystem.h>

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
	yae::Vector3 euler = yae::math::euler(*_q) * R2D;
	bool result = DragVector(_label, &euler, _speed);
	*_q = yae::Quaternion::FromEuler(euler * D2R);
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

struct ArrayEditFunctions
{
	const mirror::Type* (*getSubType)(const mirror::Type* _arrayType);
	u32 (*getSize)(const mirror::Type* _arrayType, void* _arrayPtr);
	void (*setSize)(const mirror::Type* _arrayType, void* _arrayPtr, u32 _size);
	void* (*getDataAt)(const mirror::Type* _arrayType, void* _arrayPtr, u32 _index);
	void (*erase)(const mirror::Type* _arrayType, void* _arrayPtr, u32 _index, u32 _count);
	void (*swap)(const mirror::Type* _arrayType, void* _arrayPtr, u32 _indexA, u32 _indexB);
};

bool EditArray(const mirror::Type* _arrayType, const char* _name, void* _arrayPtr, const ArrayEditFunctions& _functions)
{
	YAE_ASSERT(_functions.getSize != nullptr);
	YAE_ASSERT(_functions.getSubType != nullptr);
	YAE_ASSERT(_functions.getDataAt != nullptr);

	u32 size = _functions.getSize(_arrayType, _arrayPtr);
	const mirror::Type* subType = _functions.getSubType(_arrayType);

	auto move = [_arrayType, _functions, _arrayPtr](u32 _startIndex, u32 _endIndex)
	{
		YAE_ASSERT(_functions.swap != nullptr);

		if (_startIndex == _endIndex)
			return;

		i32 sign = yae::math::sign((i32)_endIndex - (i32)_startIndex);

		for (i32 i = (i32)_startIndex; i != (i32)_endIndex; i += sign)
		{
			_functions.swap(_arrayType, _arrayPtr, i, i+sign);
		}
	};

	bool result = false;
	if (ImGui::TreeNodeEx(_arrayPtr, 0, "%s[%d]", _name, size))
	{
		int flags = 0;
		float tableWidth = GetContentRegionAvail().x;
		float dropIntervalHeight = 8.0f;
		if (ImGui::BeginTable(_name, 2, flags))
        {
            ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed, 50.0f);
            ImGui::TableSetupColumn("Data", ImGuiTableColumnFlags_WidthStretch);

            if (size > 0 && _functions.swap != nullptr)
            {
            	ImGui::TableNextRow();
	            ImGui::TableSetColumnIndex(0);
	            ImGui::Dummy(ImVec2(tableWidth, dropIntervalHeight));
	            if (ImGui::BeginDragDropTarget())
	            {
	            	if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(_name))
	                {
	                    YAE_ASSERT(payload->DataSize == sizeof(u32));
	                    u32 sourceIndex = *(u32*)payload->Data;
	                    move(sourceIndex, 0);
	                }
	                ImGui::EndDragDropTarget();
	            }	
            }

            i32 shouldErase = -1;
            for (u32 row = 0; row < size; row++)
    		{
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("[%d]", row);
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
                }

                if (_functions.swap != nullptr)
                {
                	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
	                {
	                    ImGui::SetDragDropPayload(_name, &row, sizeof(row));
	                    ImGui::Text("Move [%d]", row);
	                	ImGui::EndDragDropSource();
	                }
                }

                if (_functions.erase != nullptr)
                {
                	ImGui::SameLine();
                	ImGui::PushID(row);
                	if (ImGui::Button("-"))
                	{
                		shouldErase = (i32)row;
                	}
                	ImGui::PopID();
                }
                ImGui::TableSetColumnIndex(1);
                void* data = _functions.getDataAt(_arrayType, _arrayPtr, row);
                ImGui::PushID(data);
                result = EditMirrorType("", data, subType) || result;
                ImGui::PopID();

                if (_functions.swap != nullptr)
	            {
	            	ImGui::TableNextRow();
		            ImGui::TableSetColumnIndex(0);
		            ImGui::Dummy(ImVec2(tableWidth, dropIntervalHeight));
		            if (ImGui::BeginDragDropTarget())
		            {
		            	if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(_name))
		                {
		                    YAE_ASSERT(payload->DataSize == sizeof(u32));
		                    u32 sourceIndex = *(u32*)payload->Data;
		                    u32 targetIndex = sourceIndex > row ? row + 1 : row;
		                    move(sourceIndex, targetIndex);
		                }
		                ImGui::EndDragDropTarget();
		            }	
	            }
            }

            if (shouldErase >= 0)
            {
                _functions.erase(_arrayType, _arrayPtr, shouldErase, 1);
            }

            if (_functions.setSize != nullptr)
            {
            	ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
            	if (ImGui::Button("+"))
            	{
            		_functions.setSize(_arrayType, _arrayPtr, size + 1);
            	}
            }

            ImGui::EndTable();
        }

		ImGui::TreePop();
	}
	return result;
}


bool EditMirrorType(const char* _name, void* _data, const mirror::Type* _type)
{
	using namespace yae;
	const float v_speed = 0.2f;

	if (_type == mirror::GetType<String>())
	{
		String* stringPtr = (String*)_data;
		return ImGui::InputText(_name, stringPtr);
	}
	else if (_type == mirror::GetType<Vector2>())
	{
		Vector2* vectorPtr = (Vector2*)_data;
		return ImGui::DragVector(_name, vectorPtr, v_speed);
	}
	else if (_type == mirror::GetType<Vector3>())
	{
		Vector3* vectorPtr = (Vector3*)_data;
		return ImGui::DragVector(_name, vectorPtr, v_speed);
	}
	else if (_type == mirror::GetType<Vector4>())
	{
		Vector4* vectorPtr = (Vector4*)_data;
		return ImGui::DragVector(_name, vectorPtr, v_speed);
	}
	else if (_type == mirror::GetType<Quaternion>())
	{
		Quaternion* quatPtr = (Quaternion*)_data;
		return ImGui::DragRotation(_name, quatPtr, v_speed);
	}
	else if (_type == mirror::GetType<Matrix3>())
	{
		Matrix3* matrixPtr = (Matrix3*)_data;
		return ImGui::DragMatrix(_name, matrixPtr, v_speed);
	}
	else if (_type == mirror::GetType<Matrix4>())
	{
		Matrix4* matrixPtr = (Matrix4*)_data;
		return ImGui::DragMatrix(_name, matrixPtr, v_speed);
	}
	else if (_type == mirror::GetType<Matrix4>())
	{
		Matrix4* matrixPtr = (Matrix4*)_data;
		return ImGui::DragMatrix(_name, matrixPtr, v_speed);
	}
	else if (_type->isCustomType("Array"))
	{
		ArrayEditFunctions functions = {};
		functions.getSubType = [](const mirror::Type* _arrayType) -> const mirror::Type*
		{
			return ((mirror::ArrayType*)_arrayType)->getSubType();
		};
		functions.getSize = [](const mirror::Type* _arrayType, void* _arrayPtr)
		{
			return (u32)((mirror::ArrayType*)_arrayType)->getSize(_arrayPtr);
		};
		functions.setSize = [](const mirror::Type* _arrayType, void* _arrayPtr, u32 _size)
		{
			((mirror::ArrayType*)_arrayType)->setSize(_arrayPtr, _size);
		};
		functions.getDataAt = [](const mirror::Type* _arrayType, void* _arrayPtr, u32 _index)
		{
			return ((mirror::ArrayType*)_arrayType)->getDataAt(_arrayPtr, _index);
		};
		functions.erase = [](const mirror::Type* _arrayType, void* _arrayPtr, u32 _index, u32 _count)
		{
			((mirror::ArrayType*)_arrayType)->erase(_arrayPtr, _index, _count);
		};
		functions.swap = [](const mirror::Type* _arrayType, void* _arrayPtr, u32 _indexA, u32 _indexB)
		{
			((mirror::ArrayType*)_arrayType)->swap(_arrayPtr, _indexA, _indexB);
		};
		return EditArray(_type, _name, _data, functions);
	}
	else if (_type->isCustomType("ResourcePtr"))
	{
		ResourceID* valuePtr = (ResourceID*) _data;
		const mirror::ResourcePtrType* resourcePtrType = (const mirror::ResourcePtrType*)_type;
		const mirror::Class* resourceClass = resourcePtrType->getSubType()->asClass();
		Resource* resource = valuePtr->get();
		String256 currentValueString = (resource == nullptr) ? "None" : filesystem::getFileName(resource->getName());

		bool modified = false;
		if (ImGui::BeginCombo(_name, currentValueString.c_str()))
		{
			if (ImGui::Selectable("None", resource == nullptr))
			{
				*valuePtr = ResourceID::INVALID_ID;
				modified = true;
			}
			const DataArray<Resource*>& resources = resourceManager().getResourcesByType(resourceClass->getTypeID());
			for (Resource* selectableResource : resources)
			{
	    		String256 name = filesystem::getFileName(selectableResource->getName());
				if (ImGui::Selectable(name.c_str(), selectableResource == resource))
				{
					*valuePtr = selectableResource->getID();
					modified = true;
				}
			}
			ImGui::EndCombo();
		}
		return modified;
	}

	switch(_type->getTypeInfo())
	{
	case mirror::TypeInfo_bool:
		{
			return ImGui::Checkbox(_name, (bool*)_data);
		}
		break;

	case mirror::TypeInfo_char:
		{
			char* charPtr = (char*)_data;
			char buf[2] = {*charPtr, 0};
			bool result = ImGui::InputText(_name, buf, 2);
			*charPtr = buf[0];
			return result;
		}
		break;

	case mirror::TypeInfo_int8:
		{
			i8* dataPtr = (i8*)_data;
			int value = *dataPtr;
			bool result = DragInt(_name, &value, v_speed, std::numeric_limits<i8>::lowest(), std::numeric_limits<i8>::max());
			*dataPtr = value;
			return result;
		}
		break;

	case mirror::TypeInfo_int16:
		{
			i16* dataPtr = (i16*)_data;
			int value = *dataPtr;
			bool result = DragInt(_name, &value, v_speed, std::numeric_limits<i16>::lowest(), std::numeric_limits<i16>::max());
			*dataPtr = value;
			return result;
		}
		break;

	case mirror::TypeInfo_int32:
		{
			i32* dataPtr = (i32*)_data;
			int value = *dataPtr;
			bool result = DragInt(_name, &value, v_speed, std::numeric_limits<i32>::lowest(), std::numeric_limits<i32>::max());
			*dataPtr = value;
			return result;
		}
		break;

	case mirror::TypeInfo_int64:
		{
			i64* dataPtr = (i64*)_data;
			int value = *dataPtr;
			bool result = DragInt(_name, &value, v_speed, std::numeric_limits<i32>::lowest(), std::numeric_limits<i32>::max());
			*dataPtr = value;
			return result;
		}
		break;

	case mirror::TypeInfo_uint8:
		{
			u8* dataPtr = (u8*)_data;
			int value = *dataPtr;
			bool result = DragInt(_name, &value, v_speed, std::numeric_limits<u8>::lowest(), std::numeric_limits<u8>::max());
			*dataPtr = value;
			return result;
		}
		break;

	case mirror::TypeInfo_uint16:
		{
			u16* dataPtr = (u16*)_data;
			int value = *dataPtr;
			bool result = DragInt(_name, &value, v_speed, std::numeric_limits<u16>::lowest(), std::numeric_limits<u16>::max());
			*dataPtr = value;
			return result;
		}
		break;

	case mirror::TypeInfo_uint32:
		{
			u32* dataPtr = (u32*)_data;
			int value = *dataPtr;
			bool result = DragInt(_name, &value, v_speed, std::numeric_limits<u32>::lowest(), std::numeric_limits<i32>::max());
			*dataPtr = value;
			return result;
		}
		break;

	case mirror::TypeInfo_uint64:
		{
			u64* dataPtr = (u64*)_data;
			int value = *dataPtr;
			bool result = DragInt(_name, &value, v_speed, std::numeric_limits<u64>::lowest(), std::numeric_limits<i32>::max());
			*dataPtr = value;
			return result;
		}
		break;

	case mirror::TypeInfo_float:
		{
			float* dataPtr = (float*)_data;
			float value = *dataPtr;
			bool result = DragFloat(_name, &value, v_speed, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			*dataPtr = value;
			return result;
		}
		break;

	case mirror::TypeInfo_double:
		{
			double* dataPtr = (double*)_data;
			float value = *dataPtr;
			bool result = DragFloat(_name, &value, v_speed, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			*dataPtr = value;
			return result;
		}
		break;

	case mirror::TypeInfo_Enum:
		{
			const mirror::Enum* enumType = _type->asEnum();
			const mirror::Type* subType = enumType->getSubType();
			if (subType == nullptr)
			{
				subType = mirror::GetType<int>();
			}
			YAE_ASSERT(subType != nullptr);

			switch(subType->getTypeInfo())
			{
				case mirror::TypeInfo_int8: return imgui_mirrorEnumCombo<i8>(_name, _data, enumType);
				case mirror::TypeInfo_int16: return imgui_mirrorEnumCombo<i16>(_name, _data, enumType);
				case mirror::TypeInfo_int32: return imgui_mirrorEnumCombo<i32>(_name, _data, enumType);
				case mirror::TypeInfo_int64: return imgui_mirrorEnumCombo<i64>(_name, _data, enumType);
				case mirror::TypeInfo_uint8: return imgui_mirrorEnumCombo<u8>(_name, _data, enumType);
				case mirror::TypeInfo_uint16: return imgui_mirrorEnumCombo<u16>(_name, _data, enumType);
				case mirror::TypeInfo_uint32: return imgui_mirrorEnumCombo<u32>(_name, _data, enumType);
				case mirror::TypeInfo_uint64: return imgui_mirrorEnumCombo<u64>(_name, _data, enumType);
				default: YAE_ASSERT("Not an enum type"); return false;
			}
		}
		break;

	case mirror::TypeInfo_FixedSizeArray:
		{
			ArrayEditFunctions functions = {};
			functions.getSubType = [](const mirror::Type* _arrayType) -> const mirror::Type*
			{
				return _arrayType->asFixedSizeArray()->getSubType();
			};
			functions.getSize = [](const mirror::Type* _arrayType, void*)
			{
				return (u32)_arrayType->asFixedSizeArray()->getElementCount();
			};
			functions.getDataAt = [](const mirror::Type* _arrayType, void* _arrayPtr, u32 _index)
			{
				return _arrayType->asFixedSizeArray()->getDataAt(_arrayPtr, _index);
			};
			functions.swap = [](const mirror::Type* _arrayType, void* _arrayPtr, u32 _indexA, u32 _indexB)
			{
				const mirror::FixedSizeArray* fixedSizeArrayType = _arrayType->asFixedSizeArray();
				size_t elementSize = fixedSizeArrayType->getSubType()->getSize();
				void* tempBuffer = scratchAllocator().allocate(elementSize);
				memcpy(tempBuffer, fixedSizeArrayType->getDataAt(_arrayPtr, _indexB), elementSize);
				memcpy(fixedSizeArrayType->getDataAt(_arrayPtr, _indexB), fixedSizeArrayType->getDataAt(_arrayPtr, _indexA), elementSize);
				memcpy(fixedSizeArrayType->getDataAt(_arrayPtr, _indexA), tempBuffer, elementSize);
				scratchAllocator().deallocate(tempBuffer);
			};
			return EditArray(_type, _name, _data, functions);
		}
		break;

	case mirror::TypeInfo_Class:
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
			ImGui::Text("%s: Unsupported type %s", _name, mirror::TypeInfoToString(_type->getTypeInfo()));
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
