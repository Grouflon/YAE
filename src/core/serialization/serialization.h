#pragma once

#include <core/serialization/Serializer.h>

#include <mirror/mirror.h>

namespace yae {
namespace serialization {

// @NOTE(remi): Enum classes are a pain to do bit masks with
enum SerializationFlags
{
	SF_IGNORE_MISSING_KEYS = 1u << 0,			// Do not return false when trying to read a missing key
	SF_ENUM_BY_NAME = 1u << 1,					// Serialize enums as strings instead of integers
	SF_ENUM_IGNORE_UNKNOWN_VALUES = 1u << 2,	// If an enum value is unknown, ignore it
};

const u32 DEFAULT_SERIALIZATION_FLAGS = SF_IGNORE_MISSING_KEYS|SF_ENUM_BY_NAME;

CORE_API bool serializeMirrorType(Serializer* _serializer, void* _value, const mirror::Type* _type, const char* _key = nullptr, u32 _flags = DEFAULT_SERIALIZATION_FLAGS);
CORE_API bool serializeClassInstance(Serializer* _serializer, void* _instance, const mirror::Class* _class, const char* _key = nullptr, u32 _flags = DEFAULT_SERIALIZATION_FLAGS);
CORE_API bool serializeClassInstanceMembers(Serializer* _serializer, void* _instance, const mirror::Class* _class, const char* _key = nullptr, u32 _flags = DEFAULT_SERIALIZATION_FLAGS);

template <typename T>
bool serializeMirrorType(Serializer* _serializer, T& _value, const char* _key = nullptr, u32 _flags = DEFAULT_SERIALIZATION_FLAGS)
{
	mirror::Type* type = mirror::GetType(_value);
	if (type == nullptr)
		return false;
	
	return serializeMirrorType(_serializer, &_value, type, _key, _flags);
}

template <typename T>
bool serializeMirrorType(Serializer* _serializer, T* _value, const char* _key = nullptr, u32 _flags = DEFAULT_SERIALIZATION_FLAGS)
{
	return serializeMirrorType(_serializer, *_value, _key, _flags);
}


} // namespace serialization
} // namespace yae
