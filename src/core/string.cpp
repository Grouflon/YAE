#include "string.h"

#include <core/math.h>
#include <core/string.h>

#include <cctype>

namespace yae {
namespace string {

const size_t INVALID_POS = size_t(-1);

size_t safeCopyToBuffer(char* _destination, const char* _source, size_t _destinationSize)
{
	u32 sourceLength = strlen(_source);
	u32 copiedLength = math::min(sourceLength, u32(_destinationSize) - 1);
	strncpy(_destination, _source, copiedLength);
	_destination[copiedLength] = 0;
	return copiedLength;
}


bool startsWith(const char* _str, const char* _searchedString)
{
	if (_str == nullptr || _searchedString == nullptr)
		return false;

	size_t length = strlen(_str);
	size_t searchedLength = strlen(_searchedString);
	if (searchedLength > length)
		return false;

	for (size_t i = 0; i < searchedLength; ++i)
	{
		if (_str[i] != _searchedString[i])
			return false;
	}
	return true;
}

bool startsWith(const String& _str, const char* _searchedString)
{
	return startsWith(_str.c_str(), _searchedString);
}

size_t find(const char* _str, const char* _searchedString, size_t _startPosition)
{
	if (_str == nullptr || _searchedString == nullptr)
		return INVALID_POS;

	size_t length = strlen(_str);
	if (_startPosition >= length)
		return INVALID_POS;

	const char* ptr = strstr(_str + _startPosition, _searchedString);
	if (ptr == nullptr)
		return INVALID_POS;

	return size_t(ptr - _str);
}

size_t find(const String& _str, const char* _searchedString, size_t _startPosition)
{
	return find(_str.c_str(), _searchedString, _startPosition);
}

String replace(const char* _str, const char* _toReplace, const char* _replacement)
{
	String result = _str;
	size_t toReplaceLen = strlen(_toReplace);
	size_t replacementLen = strlen(_replacement);
	size_t pos = find(result, _toReplace);
	while (pos != INVALID_POS)
	{
		result = replace(result, pos, toReplaceLen, _replacement);
		pos = find(result, _toReplace, pos + replacementLen);
	}
	return result;
}

String replace(String& _str, const char* _toReplace, const char* _replacement)
{
	return replace(_str.c_str(), _toReplace, _replacement);
}

String replace(const char* _str, size_t _position, size_t _count, const char* _replacement)
{
	String result = _str;
	if (result.size() == 0)
		return result;

	size_t baseLength = result.size();
	size_t count = std::min(baseLength - _position, _count);
	size_t replacementLen = strlen(_replacement);

	if (_count < replacementLen) // if replacement string is longer, resize before
	{
		result.resize(baseLength + replacementLen - count);
	}

	char* bufferPosition = result.data() + _position;
	memcpy(bufferPosition + replacementLen, bufferPosition + count, baseLength - (_position + count));
	memcpy(bufferPosition, _replacement, replacementLen);

	if (_count > replacementLen) // if replacement string is shorted, resize after
	{
		result.resize(baseLength + replacementLen - count);
	}

	return result;
}

String replace(const String& _str, size_t _position, size_t _count, const char* _replacement)
{
	return replace(_str.c_str(), _position, _count, _replacement);
}

String slice(const char* _str, size_t _startPosition, size_t _count)
{
	String result = String(&scratchAllocator());
	size_t length = strlen(_str);
	if (_startPosition > length)
		return result;

	_count = std::min(_count, length - _startPosition);
	result.resize(_count);
	memcpy(result.data(), _str + _startPosition, _count);
	return result;
}

String slice(const String& _str, size_t _startPosition, size_t _count)
{
	return slice(_str.c_str(), _startPosition, _count);
}

String toLowerCase(const char* _str)
{
	String result = _str;
	for (u32 i = 0; i < result.size(); ++i)
	{
		result[i] = std::tolower(result[i]);
	}
	return result;
}

String toLowerCase(const String& _str)
{
	return toLowerCase(_str.c_str());
}

String toUpperCase(const char* _str)
{
	String result = _str;
	for (u32 i = 0; i < result.size(); ++i)
	{
		result[i] = std::toupper(result[i]);
	}
	return result;
}

String toUpperCase(const String& _str)
{
	return toUpperCase(_str.c_str());
}

} // namespace string
} // namespace yae
