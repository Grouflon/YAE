#include "hash.h"

#include <cstring>

namespace yae {

StringHash::StringHash()
	: m_hash(0u)
#if DEBUG_STRINGHASH
	, m_string(nullptr)
#endif
{

}

StringHash::StringHash(const char* _str)
{
	m_hash = hash::hashString(_str);

#if DEBUG_STRINGHASH
	m_string = g_stringHashRepository.registerStringHash(m_hash, _str);
#endif
}



StringHash::~StringHash()
{
}



bool StringHash::operator==(const StringHash& _rhs) const
{
	return m_hash == _rhs.m_hash;
}



bool StringHash::operator==(u32 _rhs) const
{
	return m_hash == _rhs;
}



bool StringHash::operator!=(const StringHash& _rhs) const
{
	return m_hash != _rhs.m_hash;
}



bool StringHash::operator<(const StringHash& _rhs) const
{
	return m_hash < _rhs.m_hash;
}



bool StringHash::operator>(const StringHash& _rhs) const
{
	return m_hash > _rhs.m_hash;
}



bool StringHash::operator<=(const StringHash& _rhs) const
{
	return m_hash <= _rhs.m_hash;
}



bool StringHash::operator>=(const StringHash& _rhs) const
{
	return m_hash >= _rhs.m_hash;
}


u32 StringHash::operator%(u32 _rhs) const
{
	return m_hash % _rhs;
}


#if DEBUG_STRINGHASH
const char* StringHashRepository::registerStringHash(u32 _hash, const char* _string)
{
	auto it = m_stringMap.find(_hash);
	if (it == m_stringMap.end())
	{
		auto ret = m_stringMap.insert(std::pair<u32, std::string>(_hash, _string));
		YAE_ASSERT(ret.second);
		return ret.first->second.c_str();
	}
	else
	{
		YAE_ASSERT(strcmp(_string, it->second.c_str()) == 0); // Hash collision !
		return it->second.c_str();
	}
}



StringHashRepository g_stringHashRepository;
#endif



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



#if DEBUG_STRINGHASH
namespace std {

size_t hash<yae::StringHash>::operator()(const yae::StringHash& value) const
{
	return value.getHash();
}

} // namespace std
#endif
