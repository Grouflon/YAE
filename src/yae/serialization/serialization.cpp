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
			return serializeClassInstance(_serializer, _value, clss, _key, _flags);
		}

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

	if (!_serializer->endSerializeObject())
		return false;
	return true;
}

} // namespace serialization
} // namespace yae
