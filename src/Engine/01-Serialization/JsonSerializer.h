#pragma once

#include <stack>

#include <export.h>

#include <00-Type/IntTypes.h>
#include <00-Memory/Allocator.h>

struct json_value_s;
struct json_string_s;
struct json_object_s;
struct json_object_element_s;

namespace mirror { class TypeDesc; }

namespace yae {

class YAELIB_API JsonSerializer
{
public:
	JsonSerializer(Allocator* _allocator = nullptr);
	~JsonSerializer();

	void beginRead(const void* _buffer, size_t _bufferSize);
	void endRead();

	void beginWrite();
	void endWrite(const void** _data, size_t* _dataSize);

	void serialize(void* _data, const mirror::TypeDesc* _type);
private:
	void _write(void* _data, const mirror::TypeDesc* _type);
	void _read(void* _data, const mirror::TypeDesc* _type);

	// json helpers
	json_value_s* _allocateValue();
	void _allocateNullPayload(json_value_s* _value);
	void _allocateBoolPayload(json_value_s* _value, bool _payload);
	void _allocateNumberPayload(json_value_s* _value, i64 _payload);
	void _allocateNumberPayload(json_value_s* _value, u64 _payload);
	void _allocateNumberPayload(json_value_s* _value, double _payload);
	json_string_s* _allocateString(const char* _str);
	void _deallocateString(json_string_s* _str);
	void _allocateStringPayload(json_value_s* _value, const char* _payload);
	json_object_s* _allocateObjectPayload(json_value_s* _value);
	json_object_element_s* _allocateObjectElement(json_object_s* _object, const char* _name, json_object_element_s* _lastElement = nullptr);
	void _deallocateValue(json_value_s* _value);
	void _deallocatePayload(json_value_s* _value);

	bool m_isReading = false;
	json_value_s* m_root = nullptr;
	std::stack<json_value_s*> m_valueStack;

	void* m_writeData = nullptr;
	size_t m_writeDataSize = 0;

	Allocator* m_allocator = nullptr;
};

}
