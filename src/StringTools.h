#pragma once

#include <memory>
#include <string>

namespace yae {

template<typename ... Args>
std::string StringFormat( const char* _fmt, Args ..._args )
{
    size_t size = snprintf(nullptr, 0, _fmt, _args...) + 1; // Extra space for '\0'
    YAE_ASSERT(size > 0);
    std::unique_ptr<char[]> buf( new char[size] );
    snprintf(buf.get(), size, _fmt, _args...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

} // namespace yae
