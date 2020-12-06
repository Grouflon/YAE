#include "HashTools.h"

#include <cstring>

#define OFFSET_BASIS	2166136261
#define FNV_PRIME		16777619

namespace yae {

uint32_t Hash32(const void* _data, size_t _size)
{
	if (_size == 0)
	{
		return 0;
	}

	// FNV-1a algorithm
	// http://isthe.com/chongo/tech/comp/fnv/
	uint32_t hash = OFFSET_BASIS;
	const uint8_t* buf = static_cast<const uint8_t*>(_data);
	for (uint32_t i = 0u; i < _size; i++)
	{
		hash = hash * FNV_PRIME;
		hash = hash ^ *buf;
		++buf;
	}
	return hash;
}

uint32_t HashString(const char* _str)
{
	return Hash32(_str, strlen(_str));
}

} // namespace yae
