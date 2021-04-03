#pragma once

#include <unordered_map>
#include <string.h>

#include <types.h>
#include <yae_string.h>

#ifndef DEBUG_STRINGHASH
	#ifdef YAE_DEBUG
		#define DEBUG_STRINGHASH 1
	#else
		#define DEBUG_STRINGHASH 0
	#endif
#endif


namespace yae {

class YAELIB_API StringHash
{
public:
	StringHash(const char* _str);
	~StringHash();

	u32 getHash() const { return m_hash; }

	bool operator==(const StringHash& _rhs) const;
	bool operator==(u32 _rhs) const;
	bool operator!=(const StringHash& _rhs) const;
	bool operator< (const StringHash& _rhs) const;
	bool operator> (const StringHash& _rhs) const;
	bool operator<=(const StringHash& _rhs) const;
	bool operator>=(const StringHash& _rhs) const;

private:
	u32 m_hash;
#if DEBUG_STRINGHASH
	const char*	m_string;
#endif
};



#if DEBUG_STRINGHASH
class StringHashRepository
{
public:
	const char* registerStringHash(u32 _hash, const char* _string);

private:
	std::unordered_map<u32, std::string> m_stringMap;
};

extern StringHashRepository g_stringHashRepository;
#endif


namespace hash {

YAELIB_API u32 hash32(const void* _data, size_t _size);
YAELIB_API u32 hashString(const char* _str);

} // namespace hash

} // namespace yae

namespace std {
template <> struct YAELIB_API hash<yae::StringHash> { size_t operator()(const yae::StringHash& value) const; };
} // namespace std
