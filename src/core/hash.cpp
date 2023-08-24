#include "hash.h"

#include <cstring>

namespace yae {
namespace hash {

#define OFFSET_BASIS	2166136261
#define FNV_PRIME		16777619

u32 hash32(const void* _data, size_t _size)
{
	if (_size == 0)
	{
		return 0;
	}

	// FNV-1a algorithm
	// http://isthe.com/chongo/tech/comp/fnv/
	u32 hash = OFFSET_BASIS;
	const u8* buf = static_cast<const u8*>(_data);
	for (u32 i = 0u; i < _size; i++)
	{
		hash = hash * FNV_PRIME;
		hash = hash ^ *buf;
		++buf;
	}
	return hash;
}


u32 hashString(const char* _str)
{
	return hash32(_str, strlen(_str));
}

} // namespace hash
} // namespace yae
