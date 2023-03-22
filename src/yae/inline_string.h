#pragma once

#include <yae/types.h>
#include <yae/string_types.h>
#include <yae/memory.h>

namespace yae {

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

} // namespace yae
