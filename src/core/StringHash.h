#pragma once

#include <core/types.h>

#ifndef DEBUG_STRINGHASH
	#ifdef YAE_DEBUG
		#define DEBUG_STRINGHASH 1
	#else
		#define DEBUG_STRINGHASH 0
	#endif
#endif

namespace yae {

class String;

class CORE_API StringHash
{
public:
	StringHash();
	StringHash(const char* _str);
	StringHash(const String& _str);
	StringHash(u32 _hash);

	u32 getHash() const { return m_hash; }

	bool operator==(const StringHash& _rhs) const;
	bool operator==(u32 _rhs) const;
	bool operator!=(const StringHash& _rhs) const;
	bool operator< (const StringHash& _rhs) const;
	bool operator> (const StringHash& _rhs) const;
	bool operator<=(const StringHash& _rhs) const;
	bool operator>=(const StringHash& _rhs) const;
	u32 operator%(u32 _rhs) const;
	operator u32() const;

private:
	u32 m_hash;
#if DEBUG_STRINGHASH
	const char*	m_string;
#endif
};

} // namespace yae
