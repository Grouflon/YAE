#pragma once

#include <yae/types.h>

namespace yae {
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

size_t safeCopyToBuffer(char* _destination, const char* _source, size_t _destinationSize);

} // namespace string
} // namespace yae
