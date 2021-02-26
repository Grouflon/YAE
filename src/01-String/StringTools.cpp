#include "StringTools.h"

#include <cstring>
#include <locale>
#include <codecvt>

namespace yae {

// code found here: https://github.com/coder0xff/Plange/blob/master/source/utilities/utf.cpp
std::string Narrow(const wchar_t* _s)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convert;
	auto const p = reinterpret_cast<wchar_t const*>(_s);
	return convert.to_bytes(p, p + std::wcslen(_s));
}

std::string Narrow(const std::wstring& _s)
{
	return Narrow(_s.data());
}

std::wstring Widen(const char* _s)
{
	return Widen(std::string(_s));
}

std::wstring Widen(const std::string& _s)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convert;
	auto asInt = convert.from_bytes(_s);
	return std::wstring(reinterpret_cast<wchar_t const*>(asInt.data()), asInt.length());
}

std::string Replace(const std::string& _source, const char* _toReplace, const char* _replacement)
{
	std::string result = _source;
	ReplaceInline(result, _toReplace, _replacement);
	return result;
}

void ReplaceInline(std::string& _string, const char* _toReplace, const char* _replacement)
{
	size_t toReplaceLen = strlen(_toReplace);
	size_t replacementLen = strlen(_replacement);
	size_t pos = _string.find(_toReplace);
	while (pos != std::string::npos)
	{
		_string.replace(_string.begin() + pos, _string.begin() + pos + toReplaceLen, _replacement);
		pos = _string.find(_toReplace, pos + replacementLen);
	}
}

}
