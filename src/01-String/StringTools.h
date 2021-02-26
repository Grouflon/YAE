#pragma once

#include <memory>
#include <string>

namespace yae {

std::string Narrow(const wchar_t* _s);
std::string Narrow(const std::wstring& _s);
std::wstring Widen(const char* _s);
std::wstring Widen(const std::string& _s);

template<typename ... Args>
std::string StringFormat(const char* _fmt, Args ..._args)
{
    size_t size = snprintf(nullptr, 0, _fmt, _args...);
    YAE_ASSERT(size > 0);

	std::string result;
    result.resize(size);
    snprintf(const_cast<char*>(result.data()), size + 1, _fmt, _args...);

    return result;
}

std::string Replace(const std::string& _source, const char* _toReplace, const char* _replaceBy);
void ReplaceInline(std::string& _string, const char* _toReplace, const char* _replaceBy);

} // namespace yae
