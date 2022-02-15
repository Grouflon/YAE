#pragma once

#ifndef DEBUG_STRINGHASH
	#ifdef YAE_DEBUG
		#define DEBUG_STRINGHASH 1
	#else
		#define DEBUG_STRINGHASH 0
	#endif
#endif

#include <yae/types.h>

#if DEBUG_STRINGHASH
#include <yae/containers/HashMap.h>
#include <yae/string.h>
#endif


namespace yae {

class YAELIB_API StringHash
{
public:
	StringHash();
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
	u32 operator%(u32 _rhs) const;

private:
	u32 m_hash;
#if DEBUG_STRINGHASH
	const char*	m_string;
#endif
};



#if DEBUG_STRINGHASH
// @NOTE: Maybe this whole thing should be part of the program and be idle until the program is not initialized.
// Static initialization does not fit with the engine philosophy (allocators are not ready at static init time)

class StringHashRepository
{
public:
	StringHashRepository();
	const char* registerStringHash(u32 _hash, const char* _string);
	void clear();

private:
	HashMap<u32, MallocString> m_stringMap;
};

extern StringHashRepository g_stringHashRepository;
void clearStringHashRepository();
#endif


namespace hash {

YAELIB_API u32 hash32(const void* _data, size_t _size);
YAELIB_API u32 hashString(const char* _str);

} // namespace hash

} // namespace yae
