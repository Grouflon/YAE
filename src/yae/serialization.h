#pragma once

#include <stack>

#include <yae/types.h>

struct json_value_s;
struct json_string_s;
struct json_object_s;
struct json_object_element_s;

namespace mirror {
class TypeDesc;
}

namespace yae {

class Allocator;

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

	bool m_isReading = false;
	json_value_s* m_root = nullptr;
	std::stack<json_value_s*> m_valueStack;

	void* m_writeData = nullptr;
	size_t m_writeDataSize = 0;

	Allocator* m_allocator = nullptr;
};

} // namespace yae
