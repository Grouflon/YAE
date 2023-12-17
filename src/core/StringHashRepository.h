#pragma once

#include <core/types.h>
#include <core/StringHash.h>

#if DEBUG_STRINGHASH
#include <core/containers/HashMap.h>

namespace yae {

// @NOTE: Maybe this whole thing should be part of the program and be idle until the program is not initialized.
// Static initialization does not fit with the engine philosophy (allocators are not ready at static init time)

class StringHashRepository
{
public:
	StringHashRepository();
	const char* registerStringHash(u32 _hash, const char* _string);
	const char* getString(u32 _hash) const;

	void clear();

private:
	HashMap<u32, char*> m_stringMap;
};

extern StringHashRepository g_stringHashRepository;
void clearStringHashRepository();

} // namespace yae
#endif
