#pragma once

namespace yae {

const u32 INVALID_INDEX = ~u32(0);

template <typename T, template <typename> class ArrayType>
IDLookupBase<T, ArrayType>::IDLookupBase(Allocator* _allocator)
	: m_indices(_allocator)
	, m_data(_allocator)
	, m_freeListBegin(INVALID_INDEX)
	, m_freeListEnd(INVALID_INDEX)
{

}

template <typename T, template <typename> class ArrayType>
T* IDLookupBase<T, ArrayType>::get(ID _id)
{
	YAE_ASSERT(_id.index < m_indices.size());
	Index& i = m_indices[_id.index];
	return (T*)(size_t(i.id.innerId == _id.innerId) * size_t(m_data.begin() + i.dataIndex)); // branchless get
}


template <typename T, template <typename> class ArrayType>
ID IDLookupBase<T, ArrayType>::add(const T& _item)
{
	if (m_freeListBegin == INVALID_INDEX)
	{
		YAE_ASSERT(m_freeListEnd == INVALID_INDEX);

		Index i;
		i.id.index = m_indices.size();
		i.id.innerId = 0;
		i.dataIndex = i.id.index;
		i.next = INVALID_INDEX;

		m_indices.push_back(i);
		T& addedItem = m_data.push_back(_item);
		addedItem.m_id = i.id;
		return i.id;
	}
	else
	{
		Index& freeIndex = m_indices[m_freeListBegin];
		m_freeListBegin = freeIndex.next;
		if (m_freeListBegin == INVALID_INDEX)
			m_freeListEnd = INVALID_INDEX;

		freeIndex.dataIndex = m_data.size();
		T& addedItem = m_data.push_back(_item);
		addedItem.m_id = freeIndex.id;
		return freeIndex.id;
	}
}


template <typename T, template <typename> class ArrayType>
void IDLookupBase<T, ArrayType>::remove(ID _id)
{
	YAE_ASSERT(get(_id) != nullptr);

	Index& i = m_indices[_id.index];
	T& data = m_data[i.dataIndex];

	// swap data with the last data of the array, and then remove the last element
	data = m_data.back();
	m_indices[data.m_id.index].dataIndex = i.dataIndex;
	m_data.pop_back();

	i.dataIndex = INVALID_INDEX;
	i.next = INVALID_INDEX;
	++i.id.innerId;

	if (m_freeListEnd != INVALID_INDEX)
	{
		m_indices[m_freeListEnd].next = i.id.index;
	}
	else
	{
		YAE_ASSERT(m_freeListBegin == INVALID_INDEX);
		m_freeListBegin = i.id.index;
	}
	m_freeListEnd = i.id.index;
}


template <typename T, template <typename> class ArrayType>
void IDLookupBase<T, ArrayType>::clear()
{
	m_data.clear();
	for (Index& i : m_indices)
	{
		if (i.dataIndex != INVALID_INDEX)
		{
			i.dataIndex = INVALID_INDEX;
			i.next = INVALID_INDEX;
			++i.id.innerId;
			
			if (m_freeListEnd != INVALID_INDEX)
			{
				m_indices[m_freeListEnd].next = i.id.index;
			}
			else
			{
				YAE_ASSERT(m_freeListBegin == INVALID_INDEX);
				m_freeListBegin = i.id.index;
			}
			m_freeListEnd = i.id.index;
		}
	}
}


template <typename T, template <typename> class ArrayType>
T* IDLookupBase<T, ArrayType>::begin()
{
	return m_data.begin();
}


template <typename T, template <typename> class ArrayType>
const T* IDLookupBase<T, ArrayType>::begin() const
{
	return m_data.begin();
}


template <typename T, template <typename> class ArrayType>
T* IDLookupBase<T, ArrayType>::end()
{
	return m_data.end();
}


template <typename T, template <typename> class ArrayType>
const T* IDLookupBase<T, ArrayType>::end() const
{
	return m_data.end();
}


template <typename T, template <typename> class ArrayType>
Allocator* IDLookupBase<T, ArrayType>::allocator() const
{
	return m_data.allocator();
}

} // namespace yae
