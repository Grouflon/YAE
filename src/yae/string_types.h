#pragma once

#include <yae/types.h>

#include <cstdio>

namespace yae {

// @TODO: use explicit types of sizes (u32?)

class YAE_API String
{
public:
	static const size_t INVALID_POS;

	String(Allocator* _allocator = nullptr);
	String(const char* _str, Allocator* _allocator = nullptr);
	String(const String& _str, Allocator* _allocator = nullptr);
	~String();

	String(String&& _str);

	size_t size() const { return m_length; }
	size_t capacity() const { return m_bufferSize; }
	const char* c_str() const;
	char* data() { return m_buffer; }
	Allocator* allocator() const { return m_allocator; }

	void reserve(size_t _size);
	void resize(size_t _size, char _c = ' ');
	void clear();
	void shrink();

	size_t find(const char* _str, size_t _startPosition = 0) const;
	String& replace(const char* _toReplace, const char* _replacement);
	String& replace(size_t _position, size_t _count, const char* _replacement);
	String slice(size_t _startPosition, size_t _count) const;

	String& operator=(const char* _str);
	String& operator=(const String& _str);
	String operator+(char _char) const;
	String operator+(const char* _str) const;
	String operator+(const String& _str) const;
	String& operator+=(char _char);
	String& operator+=(const char* _str);
	String& operator+=(const String& _str);
	char& operator[](size_t _pos);
	const char& operator[](size_t _pos) const;
	bool operator==(const char* _str) const;
	bool operator==(const String& _str) const;
	bool operator!=(const char* _str) const;
	bool operator!=(const String& _str) const;

private:
	char* m_buffer = nullptr;
	size_t m_bufferSize = 0;
	size_t m_length = 0;
	Allocator* m_allocator = nullptr;
};

YAE_API String operator+(const char* _lhs, const String& _rhs);


class YAE_API MallocString : public String
{
public:
	MallocString();
	MallocString(const char* _str);
	MallocString(const String& _str);
};

} // namespace yae
