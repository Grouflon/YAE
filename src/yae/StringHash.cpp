#include "StringHash.h"

#include <yae/hash.h>

#if DEBUG_STRINGHASH
#include <yae/StringHashRepository.h>
#endif

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

StringHash::StringHash(u32 _hash)
{
	m_hash = _hash;

#if DEBUG_STRINGHASH
	m_string = g_stringHashRepository.getString(m_hash);
#endif
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

StringHash::operator u32() const
{
	return getHash();
}

} // namespace yae
