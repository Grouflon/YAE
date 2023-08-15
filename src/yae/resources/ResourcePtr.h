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

	ResourcePtr() : id(ResourceID::INVALID_ID) {}
	ResourcePtr(ResourceID _id) : id(_id) {}
	ResourcePtr(const T* _resource) { YAE_ASSERT(_resource != nullptr); id = _resource->getID(); }

	T* get() const
	{
		return mirror::Cast<T*>(id.get());
	}

	mirror::Class* getClass() const
	{
		return mirror::GetClass<T>();
	}
};

} // namespace yae