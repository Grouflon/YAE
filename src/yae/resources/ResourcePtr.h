#pragma once

#include <yae/types.h>
#include <yae/resources/ResourceID.h>

#include <mirror/mirror.h>

namespace yae
{

template <typename T>
struct ResourcePtr
{
	ResourceID id;

	Resource* get() const
	{
		return id.get();
	}

	mirror::Class* getClass() const
	{
		return mirror::GetClass<T>();
	}
};

} // namespace yae