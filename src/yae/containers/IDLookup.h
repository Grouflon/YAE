#pragma once

#include <yae/types.h>
#include <yae/containers/Array.h>

namespace yae {

const u64 INVALID_POOL_INDEX = ~u64(0);

typedef u64 PoolID;

template <typename T>
class IndexedPool
{
public:
	IndexedPool(Allocator* _allocator = nullptr);

	T* get(PoolID _id) const;
	PoolID add(const T& _item);
	bool remove(PoolID _id);
	void clear();

	// Iterators
	T* begin();
	const T* begin() const;
	T* end();
	const T* end() const;

	// Misc
	Allocator* allocator() const;

	struct Index
	{
		u32 generation;
		// @OPTIM(remi): I think that dataIndex is only used when the Index is alive and next only used when the Index is dead.
		// If that's the case we could gain a bit of memory by merging the two, but I don't know yet if that's worth it.
		u32 dataIndex;
		u32 next;
	};
	struct Data
	{
		u32 index;
		T data;
	};

	DataArray<Index> m_indices;
	Array<Data> m_data;

	u32 m_freeListBegin;
	u32 m_freeListEnd;
};

} // namespace yae

#include "IDLookup.inl"
