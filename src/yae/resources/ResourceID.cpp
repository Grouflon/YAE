#include "ResourceID.h"

#include <yae/ResourceManager.h>

namespace yae {

const ResourceID ResourceID::INVALID_ID = ResourceID(-1);

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