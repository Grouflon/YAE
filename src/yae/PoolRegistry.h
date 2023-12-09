#pragma once

#include <core/containers/HashMap.h>
#include <core/containers/Pool.h>
#include <yae/types.h>

#include <mirror/mirror.h>

namespace yae {

template <typename T>
struct YAE_API ID
{
	mirror::TypeID type;
	PoolID id;

	T* get() const;
};

class YAE_API PoolRegistry
{
public:
	template <typename T, ...Args>
	ID<T> create(Args...);

	template <typename T>
	void destroy(ID<T> _id);

	template <typename T>
	Pool<T>& getOrCreatePool();

	template <typename T>
	Pool<T>& getOrCreatePool(mirror::TypeID _type);

//private:
	HashMap<mirror::TypeID, void*> m_pools;

};

} // namespace yae
