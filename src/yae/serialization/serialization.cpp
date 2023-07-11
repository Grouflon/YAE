#include "serialization.h"

#include <yae/memory.h>
#include <yae/containers/Array.h>


namespace yae {
namespace serialization {

bool serializeMirrorType(Serializer* _serializer, void* _value, const mirror::TypeDesc* _type, const char* _key, u32 _flags)
{
	YAE_ASSERT(_type != nullptr);
	YAE_ASSERT(_value != nullptr);

	switch(_type->getType())
	{
		case mirror::Type_bool:   return _serializer->serialize(*(bool*)_value, _key);
		case mirror::Type_char:   return _serializer->serialize(*(u8*)_value, _key);
		case mirror::Type_int8:   return _serializer->serialize(*(i8*)_value, _key);
		case mirror::Type_int16:  return _serializer->serialize(*(i16*)_value, _key);
		case mirror::Type_int32:  return _serializer->serialize(*(i32*)_value, _key);
		case mirror::Type_int64:  return _serializer->serialize(*(i64*)_value, _key);
		case mirror::Type_uint8:  return _serializer->serialize(*(u8*)_value, _key);
		case mirror::Type_uint16: return _serializer->serialize(*(u16*)_value, _key);
		case mirror::Type_uint32: return _serializer->serialize(*(u32*)_value, _key);
		case mirror::Type_uint64: return _serializer->serialize(*(u64*)_value, _key);
		case mirror::Type_float:  return _serializer->serialize(*(float*)_value, _key);
		case mirror::Type_double: return _serializer->serialize(*(double*)_value, _key);

		case mirror::Type_Class:
		{
			const mirror::Class* clss = _type->asClass();
			YAE_ASSERT(clss);

			// Special route for strings
			if (clss == mirror::GetClass<String>())
			{
				return _serializer->serialize(*(String*)_value, _key);
			}

			// Simplified serialization for types that can be considered float arrays (e.g. Vectors, Quaternions...)
			const mirror::MetaData* floatArrayMetaData = clss->getMetaDataSet().findMetaData("FloatArray");
			if (floatArrayMetaData != nullptr)
			{
				u32 arraySize = (u32)floatArrayMetaData->asInt();

				if (!_serializer->beginSerializeArray(arraySize, _key))
					return _flags & SF_IGNORE_MISSING_KEYS;

				for (u32 i = 0; i < arraySize; ++i)
				{
					if (!_serializer->serialize(*((float*)_value + i)))
						return false;
				}

				if (!_serializer->endSerializeArray())
					return false;
				return true;
			}

			// Otherwise use standard reflection
			return serializeClassInstance(_serializer, _value, clss, _key, _flags);
		}

		case mirror::Type_Enum:
		{
			const mirror::Enum* enm = _type->asEnum();
			YAE_ASSERT(enm);

			mirror::TypeDesc* subType = enm->getSubType();
			if (subType == nullptr)
			{
				subType = mirror::GetTypeDesc<int>();
			}

			if (_flags & SF_ENUM_BY_NAME) // Serialize string instead of integer value
			{
				auto getValue = [](void* _value, const mirror::TypeDesc* _type)
				{
					switch(_type->getType())
					{
						case mirror::Type_int8:   return (i64)(*(i8*)_value);
						case mirror::Type_int16:  return (i64)(*(i16*)_value);
						case mirror::Type_int32:  return (i64)(*(i32*)_value);
						case mirror::Type_int64:  return (i64)(*(i64*)_value);
						case mirror::Type_uint8:  return (i64)(*(u8*)_value);
						case mirror::Type_uint16: return (i64)(*(u16*)_value);
						case mirror::Type_uint32: return (i64)(*(u32*)_value);
						case mirror::Type_uint64: return (i64)(*(u64*)_value);
						default: return i64(0);
					}
				};
				auto setValue = [](void* _value, const mirror::TypeDesc* _type, i64 _newValue)
				{
					switch(_type->getType())
					{
						case mirror::Type_int8:   (*(i8*)(_value)) = (i8)_newValue;
						case mirror::Type_int16:  (*(i16*)(_value)) = (i16)_newValue;
						case mirror::Type_int32:  (*(i32*)(_value)) = (i32)_newValue;
						case mirror::Type_int64:  (*(i64*)(_value)) = (i64)_newValue;
						case mirror::Type_uint8:  (*(u8*)(_value)) = (u8)_newValue;
						case mirror::Type_uint16: (*(u16*)(_value)) = (u16)_newValue;
						case mirror::Type_uint32: (*(u32*)(_value)) = (u32)_newValue;
						case mirror::Type_uint64: (*(u64*)(_value)) = (u64)_newValue;
						default: return;
					}
				};

				switch(_serializer->getMode())
				{
					case SerializationMode::READ:
					{
						String valueString = String(&scratchAllocator());
						if (!_serializer->serialize(valueString, _key))
							return false;

						i64 value = 0;
						bool result = enm->getValueFromString(valueString.c_str(), value);
						if (!result && !(_flags & SF_ENUM_IGNORE_UNKNOWN_VALUES))
							return false;

						setValue(_value, subType, value);
					}
					break;

					case SerializationMode::WRITE:
					{
						i64 value = getValue(_value, subType);
						const char* valueStringLiteral;
						bool result = enm->getStringFromValue(value, valueStringLiteral);
						if (!result && !(_flags & SF_ENUM_IGNORE_UNKNOWN_VALUES))
							return false;

						String valueString = String(valueStringLiteral, &scratchAllocator());
						return _serializer->serialize(valueString, _key);
					}
					break;

					default: YAE_ASSERT(false); break;
				}

				
			}
			else
			{
				return serializeMirrorType(_serializer, _value, subType, _key, _flags);
			}
		}
		break;

		case mirror::Type_FixedSizeArray:
		{
			const mirror::FixedSizeArray* fixedSizeArrayType = _type->asFixedSizeArray();
			YAE_ASSERT(fixedSizeArrayType != nullptr);
			u32 arraySize = fixedSizeArrayType->getElementCount();
			if (!_serializer->beginSerializeArray(arraySize, _key))
				return _flags & SF_IGNORE_MISSING_KEYS;

			const mirror::TypeDesc* subType = fixedSizeArrayType->getSubType();
			for (u32 i = 0; i < arraySize; ++i)
			{
				void* valuePointer = (void*)(size_t(_value) + (i * subType->getSize()));
				bool success = serializeMirrorType(_serializer, valuePointer, subType, nullptr, _flags);
				if (!success)
					return false;
			}

			if (!_serializer->endSerializeArray())
				return false;
		}
		break;

		default: YAE_ASSERT_MSG(false, "type not implemented"); break;
	}
	return false;
}

bool serializeClassInstance(Serializer* _serializer, void* _instance, const mirror::Class* _class, const char* _key, u32 _flags)
{
	if (!_serializer->beginSerializeObject(_key))
		return _flags & SF_IGNORE_MISSING_KEYS;

	if (!serializeClassInstanceMembers(_serializer, _instance, _class, _key, _flags))
		return false;

	if (!_serializer->endSerializeObject())
		return false;
	return true;
}

bool serializeClassInstanceMembers(Serializer* _serializer, void* _instance, const mirror::Class* _class, const char* _key, u32 _flags)
{
	size_t membersCount = _class->getMembersCount();
	DataArray<mirror::ClassMember*> members(&scratchAllocator());
	members.resize(membersCount);
	YAE_VERIFY(_class->getMembers(members.data(), membersCount) == membersCount);

	for (mirror::ClassMember* member : members)
	{
		void* memberPointer = member->getInstanceMemberPointer(_instance);
		bool success = serializeMirrorType(_serializer, memberPointer, member->getType(), member->getName(), _flags);

		if (!success && !(_flags & SF_IGNORE_MISSING_KEYS))
			return false;
	}
	return true;
}

} // namespace serialization
} // namespace yae
