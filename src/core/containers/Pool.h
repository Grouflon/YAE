#pragma once

#include <core/types.h>
#include <core/containers/Array.h>
#include <core/containers/PoolID.h>

namespace yae {

template <typename T>
class Pool
{
public:
	Pool(Allocator* _allocator = nullptr);

	const T* get(PoolID _id) const;
	T* get(PoolID _id);
	
	PoolID add(const T& _item);
	bool remove(PoolID _id);
	void clear();
	u32 size();

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

#include "Pool.inl"
