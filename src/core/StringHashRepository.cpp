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
	MallocString* stringPtr = m_stringMap.get(_hash);
	if (stringPtr == nullptr)
	{
		MallocString& string = m_stringMap.set(_hash, _string);
		return string.c_str();
	}
	else
	{
		YAE_ASSERT(strcmp(_string, stringPtr->c_str()) == 0); // Hash collision !
		return stringPtr->c_str();
	}
}

const char* StringHashRepository::getString(u32 _hash) const
{
	const MallocString* stringPtr = m_stringMap.get(_hash);
	if (stringPtr != nullptr)
	{
		return stringPtr->c_str();
	}
	return nullptr;
}

void StringHashRepository::clear()
{
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
