#pragma once

#include <yae/types.h>
#include <yae/containers/Array.h>

namespace yae {

struct YAELIB_API ID
{
	u32 index;
	u32 innerId;
};


template <typename T, template <typename> class ArrayType>
class IDLookupBase
{
public:
	IDLookupBase(Allocator* _allocator = nullptr);

	T* get(ID _id);
	ID add(const T& _item);
	void remove(ID _id);
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
		ID id;
		u32 dataIndex;
		u32 next;
	};

	DataArray<Index> m_indices;
	ArrayType<T> m_data;

	u32 m_freeListBegin;
	u32 m_freeListEnd;
};


template <typename T>
class IDLookup : public IDLookupBase<T, Array>
{
public:
	IDLookup(Allocator* _allocator = nullptr) : IDLookupBase(_allocator) {}
};


template <typename T>
class IDDataLookup : public IDLookupBase<T, DataArray>
{
public:
	IDDataLookup(Allocator* _allocator = nullptr) : IDLookupBase(_allocator) {}
};

} // namespace yae

#include "IDLookup.inl"
