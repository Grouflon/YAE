#pragma once

#include <yae/types.h>

namespace yae
{

struct YAE_API ResourceID
{
	static const u32 INVALID_ID = -1;

	u32 id;

	Resource* get() const;

	ResourceID();
	ResourceID(u32 _id);
	u32 operator%(u32 _rhs) const;
	operator u32() const;
};

} // namespace yae