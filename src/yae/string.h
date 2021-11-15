#pragma once

#include <yae/types.h>
#include <yae/memory.h>

namespace yae {

class Allocator;

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
	const char* c_str() const { return m_buffer; }
	char* data() { return m_buffer; }
	Allocator* allocator() const { return m_allocator; }

	void reserve(size_t _size);
	void resize(size_t _size, char _c = ' ');
	void clear();

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
	/*MallocString(const char* _str);
	MallocString(const String& _str);*/
};



template <size_t INLINE_SIZE>
class InlineString : public String
{
public:
	InlineString() : String(&m_inlineAllocator) {}
	InlineString(const char* _str) : String(&m_inlineAllocator)
	{
		String::operator=(_str);
	}
	InlineString(const String& _str) :String(&m_inlineAllocator)
	{
		String::operator=(_str);
	}

private:
	InlineAllocator<INLINE_SIZE> m_inlineAllocator;
};

typedef InlineString<32> String32;
typedef InlineString<64> String64;
typedef InlineString<128> String128;
typedef InlineString<256> String256;
typedef InlineString<512> String512;


// TOOLS
namespace string {

/*YAELIB_API std::string Narrow(const wchar_t* _s);
YAELIB_API std::string Narrow(const std::wstring& _s);
YAELIB_API std::wstring Widen(const char* _s);
YAELIB_API std::wstring Widen(const std::string& _s);

template<typename ... Args>
std::string StdStringFormat(const char* _fmt, Args ..._args)
{
    size_t size = snprintf(nullptr, 0, _fmt, _args...);
    YAE_ASSERT(size > 0);

	std::string result;
    result.resize(size);
    snprintf(const_cast<char*>(result.data()), size + 1, _fmt, _args...);

    return result;
}*/

template<typename ... Args>
String format(const char* _fmt, Args ..._args)
{
    size_t size = snprintf(nullptr, 0, _fmt, _args...);
    YAE_ASSERT(size > 0);

	String result;
    result.resize(size);
    snprintf(result.data(), size + 1, _fmt, _args...);

    return result;
}

} // namespace string

} // namespace yae
