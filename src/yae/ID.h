#pragma once

#include <yae/types.h>
#include <core/containers/Pool.h>

namespace yae {

template <typename T>
struct ID
{
	ID() = default;
	ID(PoolID _id, Pool<T>* _pool) : id(_id), pool(_pool) {}

	PoolID id = INVALID_POOL_INDEX;
	Pool<T>* pool = nullptr;

	T* get() const { return pool != nullptr ? pool->get(id) : nullptr; }
	T& operator*() const { return *get(); }
	T* operator->() const { return get(); }
	bool operator==(ID<T> _rhs) const { return id == _rhs.id && pool == _rhs.pool; }

	static const ID<T> INVALID;
};

template <typename T>
const ID<T> ID<T>::INVALID = ID<T>();

// template <typename T>
// T* resolveID(ID<T> _id)
// {
// 	static_assert(false);
// }

// template <> YAE_API SceneGraphNode* resolveID(ID<SceneGraphNode> _id);
// template <> YAE_API Scene* resolveID(ID<Scene> _id);
// template <> YAE_API Entity* resolveID(ID<Entity> _id);

} // namespace yae
