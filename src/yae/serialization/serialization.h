#pragma once

#include <yae/serialization/Serializer.h>

#include <mirror/mirror.h>

namespace yae {
namespace serialization {

// @NOTE(remi): Enum classes are a pain to do bit masks with
enum SerializationFlags
{
	SF_IGNORE_MISSING_KEYS = 1u << 0,
};

const u32 DEFAULT_SERIALIZATION_FLAGS = SF_IGNORE_MISSING_KEYS;

YAE_API bool serializeMirrorType(Serializer* _serializer, void* _value, const mirror::TypeDesc* _type, const char* _key = nullptr, u32 _flags = DEFAULT_SERIALIZATION_FLAGS);
YAE_API bool serializeClassInstance(Serializer* _serializer, void* _instance, const mirror::Class* _class, const char* _key = nullptr, u32 _flags = DEFAULT_SERIALIZATION_FLAGS);

template <typename T>
bool serializeMirrorType(Serializer* _serializer, T& _value, const char* _key = nullptr, u32 _flags = DEFAULT_SERIALIZATION_FLAGS)
{
	mirror::TypeDesc* type = mirror::GetTypeDesc(_value);
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
