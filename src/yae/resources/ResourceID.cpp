#include "ResourceID.h"

#include <yae/ResourceManager.h>

#include <mirror/mirror.h>

namespace yae {

Resource* ResourceID::get() const
{
	return resourceManager().findResource(*this);
}

ResourceID::ResourceID()
{
	*this = INVALID_ID;
}

ResourceID::ResourceID(u32 _id)
	: id(_id)
{
}

u32 ResourceID::operator%(u32 _rhs) const
{
	return id % _rhs;
}

ResourceID::operator u32() const
{
	return id;
}

} // namespace yae

MIRROR_CLASS(yae::ResourceID, SerializeType = u32)
(
);
