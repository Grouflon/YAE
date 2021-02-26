#include "StringHash.h"

#include <00-Macro/Assert.h>

#include "HashTools.h"

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

u32 StringHash::getHash() const
{
	return m_hash;
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

} // namespace yae

namespace std {

size_t hash<yae::StringHash>::operator()(const yae::StringHash& value) const
{
	return value.getHash();
}

} // namespace str
