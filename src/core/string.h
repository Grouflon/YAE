#pragma once

#include <core/types.h>

namespace yae {
namespace string {

extern const size_t INVALID_POS;

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

template<typename ... Args>
size_t format(char* _dest, size_t _destSize, const char* _fmt, Args ..._args)
{
    YAE_ASSERT(_fmt != nullptr);
    size_t size = snprintf(nullptr, 0, _fmt, _args...);
    YAE_ASSERT(size > 0);

    return snprintf(_dest, _destSize < (size+1) ? _destSize : size+1, _fmt, _args...);
}

CORE_API size_t safeCopyToBuffer(char* _destination, const char* _source, size_t _destinationSize);

CORE_API bool startsWith(const char* _str, const char* _searchedString);
CORE_API bool startsWith(const String& _str, const char* _searchedString);
CORE_API size_t find(const char* _str, const char* _searchedString, size_t _startPosition = 0);
CORE_API size_t find(const String& _str, const char* _searchedString, size_t _startPosition = 0);
CORE_API String replace(const char* _str, const char* _toReplace, const char* _replacement);
CORE_API String replace(const String& _str, const char* _toReplace, const char* _replacement);
CORE_API String replace(const char* _str, size_t _position, size_t _count, const char* _replacement);
CORE_API String replace(const String& _str, size_t _position, size_t _count, const char* _replacement);
CORE_API String slice(const char* _str, size_t _startPosition, size_t _count);
CORE_API String slice(const String& _str, size_t _startPosition, size_t _count);
CORE_API String toLowerCase(const char* _str);
CORE_API String toLowerCase(const String& _str);
CORE_API String toUpperCase(const char* _str);
CORE_API String toUpperCase(const String& _str);

} // namespace string
} // namespace yae
