#pragma once

#include <export.h>

namespace yae {

class Allocator;

class YAELIB_API String
{
public:
	static const size_t INVALID_POS;

	String(Allocator* _allocator = nullptr);
	String(const char* _str, Allocator* _allocator = nullptr);
	String(const String& _str, Allocator* _allocator = nullptr);
	~String();

	String(String&& _str);

	size_t getLength() const { return m_length; }
	const char* c_str() const { return m_buffer; }
	char* data() { return m_buffer; }

	void reserve(size_t _size);
	void resize(size_t _size, char _c = ' ');
	void clear();

	size_t find(const char* _str, size_t _startPosition = 0) const;
	String& replace(const char* _toReplace, const char* _replacement);
	String& replace(size_t _position, size_t _count, const char* _replacement);

	String& operator=(const String& _str);

private:
	char* m_buffer = nullptr;
	size_t m_bufferSize = 0;
	size_t m_length = 0;
	Allocator* m_allocator = nullptr;
};

}
