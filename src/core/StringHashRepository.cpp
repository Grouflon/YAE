#include "StringHashRepository.h"

#include <cstring>

namespace yae {

#if DEBUG_STRINGHASH
StringHashRepository::StringHashRepository()
	: m_stringMap(&mallocAllocator())
{

}

const char* StringHashRepository::registerStringHash(u32 _hash, const char* _string)
{
	char** stringPtr = m_stringMap.get(_hash);
	if (stringPtr == nullptr)
	{
		size_t stringLength = strlen(_string);
		char* buffer = (char*)malloc(stringLength + 1);
		strcpy(buffer, _string);
		m_stringMap.set(_hash, buffer);
		return buffer;
	}
	else
	{
		YAE_ASSERT(strcmp(_string, *stringPtr) == 0); // Hash collision !
		return *stringPtr;
	}
}

const char* StringHashRepository::getString(u32 _hash) const
{
	char* const* stringPtr = m_stringMap.get(_hash);
	if (stringPtr != nullptr)
	{
		return *stringPtr;
	}
	return nullptr;
}

void StringHashRepository::clear()
{
	for (auto pair : m_stringMap)
	{
		free(pair.value);
	}
	m_stringMap.clear();
	m_stringMap.shrink();
}


StringHashRepository g_stringHashRepository;

void clearStringHashRepository()
{
	g_stringHashRepository.clear();
}

#endif

} // namespace yae
