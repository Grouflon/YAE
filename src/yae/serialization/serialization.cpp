#include "serialization.h"

#include <yae/memory.h>
#include <yae/containers/Array.h>


namespace yae {
namespace serialization {

bool serializeMirrorType(Serializer* _serializer, void* _value, const mirror::Type* _type, const char* _key, u32 _flags)
{
	YAE_ASSERT(_type != nullptr);
	YAE_ASSERT(_value != nullptr);

	switch(_type->getTypeInfo())
	{
		case mirror::TypeInfo_bool:   return _serializer->serialize(*(bool*)_value, _key);
		case mirror::TypeInfo_char:   return _serializer->serialize(*(u8*)_value, _key);
		case mirror::TypeInfo_int8:   return _serializer->serialize(*(i8*)_value, _key);
		case mirror::TypeInfo_int16:  return _serializer->serialize(*(i16*)_value, _key);
		case mirror::TypeInfo_int32:  return _serializer->serialize(*(i32*)_value, _key);
		case mirror::TypeInfo_int64:  return _serializer->serialize(*(i64*)_value, _key);
		case mirror::TypeInfo_uint8:  return _serializer->serialize(*(u8*)_value, _key);
		case mirror::TypeInfo_uint16: return _serializer->serialize(*(u16*)_value, _key);
		case mirror::TypeInfo_uint32: return _serializer->serialize(*(u32*)_value, _key);
		case mirror::TypeInfo_uint64: return _serializer->serialize(*(u64*)_value, _key);
		case mirror::TypeInfo_float:  return _serializer->serialize(*(float*)_value, _key);
		case mirror::TypeInfo_double: return _serializer->serialize(*(double*)_value, _key);

		case mirror::TypeInfo_Class:
		{
			const mirror::Class* clss = _type->asClass();
			YAE_ASSERT(clss);

			// Special route for strings
			if (clss == mirror::GetClass<String>())
			{
				return _serializer->serialize(*(String*)_value, _key);
			}

			// Simplified serialization for overriden serialization types that can be considered primitive arrays (e.g. Vectors, Quaternions...)
			const mirror::MetaData* serializeTypeMetaData = clss->getMetaDataSet().findMetaData("SerializeType");
			if (serializeTypeMetaData != nullptr)
			{
				const mirror::Type* serializeType = mirror::FindTypeByName(serializeTypeMetaData->asString());
				if (serializeType == nullptr)
					return false;

				const mirror::MetaData* serializeArraySizeMetaData = clss->getMetaDataSet().findMetaData("SerializeArraySize");
				// If we have an array size, then let's serialize the type as an array
				if (serializeArraySizeMetaData != nullptr)
				{
					u32 arraySize = (u32)serializeArraySizeMetaData->asInt();
					size_t typeSize = serializeType->getSize();

					if (!_serializer->beginSerializeArray(arraySize, _key))
						return _flags & SF_IGNORE_MISSING_KEYS;

					for (u32 i = 0; i < arraySize; ++i)
					{
						if (!serializeMirrorType(_serializer, (char*)_value + i * typeSize, serializeType, nullptr, _flags))
							return false;
					}

					if (!_serializer->endSerializeArray())
						return false;
					return true;
				}
				else
				{
					return serializeMirrorType(_serializer, _value, serializeType, _key, _flags);
				}
			}

			// Otherwise use standard reflection
			return serializeClassInstance(_serializer, _value, clss, _key, _flags);
		}

		case mirror::TypeInfo_Enum:
		{
			const mirror::Enum* enm = _type->asEnum();
			YAE_ASSERT(enm);

			mirror::Type* subType = enm->getSubType();
			if (subType == nullptr)
			{
				subType = mirror::GetType<int>();
			}

			if (_flags & SF_ENUM_BY_NAME) // Serialize string instead of integer value
			{
				auto getValue = [](void* _value, const mirror::Type* _type)
				{
					switch(_type->getTypeInfo())
					{
						case mirror::TypeInfo_int8:   return (i64)(*(i8*)_value);
						case mirror::TypeInfo_int16:  return (i64)(*(i16*)_value);
						case mirror::TypeInfo_int32:  return (i64)(*(i32*)_value);
						case mirror::TypeInfo_int64:  return (i64)(*(i64*)_value);
						case mirror::TypeInfo_uint8:  return (i64)(*(u8*)_value);
						case mirror::TypeInfo_uint16: return (i64)(*(u16*)_value);
						case mirror::TypeInfo_uint32: return (i64)(*(u32*)_value);
						case mirror::TypeInfo_uint64: return (i64)(*(u64*)_value);
						default: return i64(0);
					}
				};
				auto setValue = [](void* _value, const mirror::Type* _type, i64 _newValue)
				{
					switch(_type->getTypeInfo())
					{
						case mirror::TypeInfo_int8:   (*(i8*)(_value)) = (i8)_newValue;
						case mirror::TypeInfo_int16:  (*(i16*)(_value)) = (i16)_newValue;
						case mirror::TypeInfo_int32:  (*(i32*)(_value)) = (i32)_newValue;
						case mirror::TypeInfo_int64:  (*(i64*)(_value)) = (i64)_newValue;
						case mirror::TypeInfo_uint8:  (*(u8*)(_value)) = (u8)_newValue;
						case mirror::TypeInfo_uint16: (*(u16*)(_value)) = (u16)_newValue;
						case mirror::TypeInfo_uint32: (*(u32*)(_value)) = (u32)_newValue;
						case mirror::TypeInfo_uint64: (*(u64*)(_value)) = (u64)_newValue;
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

		case mirror::TypeInfo_FixedSizeArray:
		{
			const mirror::FixedSizeArray* fixedSizeArrayType = _type->asFixedSizeArray();
			YAE_ASSERT(fixedSizeArrayType != nullptr);
			u32 arraySize = fixedSizeArrayType->getElementCount();
			if (!_serializer->beginSerializeArray(arraySize, _key))
				return _flags & SF_IGNORE_MISSING_KEYS;

			const mirror::Type* subType = fixedSizeArrayType->getSubType();
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

		case mirror::TypeInfo_Custom:
		{
			if (strcmp(_type->getCustomTypeName(), "Array") == 0)
			{
				const mirror::ArrayType* arrayType = (const mirror::ArrayType*)_type;

				u32 arraySize = arrayType->getSize(_value);
				if (!_serializer->beginSerializeArray(arraySize, _key))
					return _flags & SF_IGNORE_MISSING_KEYS;
				arrayType->setSize(_value, arraySize);

				const mirror::Type* subType = arrayType->getSubType();
				for (u32 i = 0; i < arraySize; ++i)
				{
					void* valuePointer = (void*)(size_t(arrayType->getData(_value)) + (i * subType->getSize()));
					bool success = serializeMirrorType(_serializer, valuePointer, subType, nullptr, _flags);
					if (!success)
						return false;
				}

				if (!_serializer->endSerializeArray())
					return false;
			}
		}
		break;

		default: YAE_ASSERT_MSG(false, "type not implemented"); break;
	}
	return true;
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
