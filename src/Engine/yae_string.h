#pragma once

#include <types.h>

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



class YAELIB_API MallocString : public String
{
public:
	MallocString();
	MallocString(const char* _str);
	MallocString(const String& _str);
};


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
