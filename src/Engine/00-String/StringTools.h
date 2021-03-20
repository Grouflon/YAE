#pragma once

#include <export.h>

#include <string>

#include <00-String/String.h>

namespace yae {

YAELIB_API std::string Narrow(const wchar_t* _s);
YAELIB_API std::string Narrow(const std::wstring& _s);
YAELIB_API std::wstring Widen(const char* _s);
YAELIB_API std::wstring Widen(const std::string& _s);

template<typename ... Args>
YAELIB_API std::string StdStringFormat(const char* _fmt, Args ..._args)
{
    size_t size = snprintf(nullptr, 0, _fmt, _args...);
    YAE_ASSERT(size > 0);

	std::string result;
    result.resize(size);
    snprintf(const_cast<char*>(result.data()), size + 1, _fmt, _args...);

    return result;
}

template<typename ... Args>
YAELIB_API String StringFormat(const char* _fmt, Args ..._args)
{
    size_t size = snprintf(nullptr, 0, _fmt, _args...);
    YAE_ASSERT(size > 0);

	String result;
    result.resize(size);
    snprintf(result.data(), size + 1, _fmt, _args...);

    return result;
}

YAELIB_API String Replace(const String& _source, const char* _toReplace, const char* _replaceBy);
YAELIB_API void ReplaceInline(String& _string, const char* _toReplace, const char* _replaceBy);

} // namespace yae
