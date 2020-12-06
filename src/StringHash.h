#pragma once

#include <unordered_map>
#include <string.h>
#include <stdint.h>

#ifdef _DEBUG
#define DEBUG_STRINGHASH 1
#else
#define DEBUG_STRINGHASH 0
#endif

namespace yae {

class StringHash
{
public:
	StringHash(const char* _str);
	~StringHash();

	uint32_t getHash() const;

	bool operator==(const StringHash& _rhs) const;
	bool operator==(uint32_t _rhs) const;
	bool operator!=(const StringHash& _rhs) const;
	bool operator< (const StringHash& _rhs) const;
	bool operator> (const StringHash& _rhs) const;
	bool operator<=(const StringHash& _rhs) const;
	bool operator>=(const StringHash& _rhs) const;

private:
	uint32_t m_hash;
#if DEBUG_STRINGHASH
	const char*	m_string;
#endif
};

#if DEBUG_STRINGHASH
class StringHashRepository
{
public:
	const char* registerStringHash(uint32_t _hash, const char* _string);

private:
	std::unordered_map<uint32_t, std::string> m_stringMap;
};

extern StringHashRepository g_stringHashRepository;
#endif

} // namespace yae

namespace std {
template <> struct hash<yae::StringHash> { size_t operator()(const yae::StringHash& value) const; };
} // namespace std
