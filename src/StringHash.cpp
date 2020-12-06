#include "StringHash.h"

#include <Assert.h>
#include <HashTools.h>

namespace yae {

StringHash::StringHash(const char* _str)
{
	m_hash = HashString(_str);

#if DEBUG_STRINGHASH
	m_string = g_stringHashRepository.registerStringHash(m_hash, _str);
#endif
}

StringHash::~StringHash()
{
}

uint32_t StringHash::getHash() const
{
	return m_hash;
}

bool StringHash::operator==(const StringHash& _rhs) const
{
	return m_hash == _rhs.m_hash;
}

bool StringHash::operator==(uint32_t _rhs) const
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

#if DEBUG_STRINGHASH
const char* StringHashRepository::registerStringHash(uint32_t _hash, const char* _string)
{
	auto it = m_stringMap.find(_hash);
	if (it == m_stringMap.end())
	{
		auto ret = m_stringMap.insert(std::pair<uint32_t, std::string>(_hash, _string));
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

} // namespace yae

namespace std {

size_t hash<yae::StringHash>::operator()(const yae::StringHash& value) const
{
	return value.getHash();
}

} // namespace str
