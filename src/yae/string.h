#pragma once

#include <yae/types.h>

#include <cstdio>

namespace yae {

// @TODO: use explicit types of sizes (u32?)

class YAELIB_API String
{
public:
	static const size_t INVALID_POS;

	String(Allocator* _allocator = nullptr);
	String(const char* _str, Allocator* _allocator = nullptr);
	String(const String& _str, Allocator* _allocator = nullptr);
	~String();

	String(String&& _str);

	size_t size() const { return m_length; }
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

private:
	char* m_buffer = nullptr;
	size_t m_bufferSize = 0;
	size_t m_length = 0;
	Allocator* m_allocator = nullptr;
};



class YAELIB_API MallocString : public String
{
public:
	MallocString();
	MallocString(const char* _str);
	MallocString(const String& _str);
};


// TOOLS
namespace string {

template<typename ... Args>
String format(const char* _fmt, Args ..._args)
{
    YAE_ASSERT(_fmt != nullptr);
    size_t size = snprintf(nullptr, 0, _fmt, _args...);
    YAE_ASSERT(size > 0);

	String result(&scratchAllocator());
    result.resize(size);
    snprintf(result.data(), size + 1, _fmt, _args...);

    return result;
}

} // namespace string

} // namespace yae
