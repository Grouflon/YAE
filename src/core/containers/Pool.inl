#pragma once

namespace yae {

const u32 INVALID_INDEX = ~u32(0);

constexpr u32 extractIndexFromId(PoolID _id)
{
	return _id & ((u64(1) << 32) - 1);
}

constexpr u32 extractGenerationFromId(PoolID _id)
{
	return u32(_id >> 32);
}

constexpr PoolID makeId(u32 _index, u32 _generation)
{
	return u64(_index) | u64(_generation) << 32;
}

template <typename T>
Pool<T>::Pool(Allocator* _allocator)
	: m_indices(_allocator)
	, m_data(_allocator)
	, m_freeListBegin(INVALID_INDEX)
	, m_freeListEnd(INVALID_INDEX)
{

}

template <typename T>
const T* Pool<T>::get(PoolID _id) const
{
	const u32 index = extractIndexFromId(_id);
	const u32 generation = extractGenerationFromId(_id);
	if (index >= m_indices.size())
		return nullptr;

	const Index& i = m_indices[index];
	if (i.generation != generation)
		return nullptr;
	
	return &(m_data.begin() + i.dataIndex)->data;
}

template <typename T>
T* Pool<T>::get(PoolID _id)
{

	return const_cast<T*>(const_cast<const Pool<T>*>(this)->get(_id));
}

template <typename T>
PoolID Pool<T>::add(const T& _item)
{
	u32 index;
	Index* indexPtr;

	if (m_freeListBegin == INVALID_INDEX)
	{
		YAE_ASSERT(m_freeListEnd == INVALID_INDEX);

		Index newIndex;
		newIndex.generation = 0;
		index = m_indices.size();
		indexPtr = &m_indices.push_back(newIndex);
	}
	else
	{
		index = m_freeListBegin;
		indexPtr = &m_indices[index];

		m_freeListBegin = indexPtr->next;
		if (m_freeListBegin == INVALID_INDEX)
			m_freeListEnd = INVALID_INDEX;

	}

	indexPtr->dataIndex = m_data.size();
	indexPtr->next = INVALID_INDEX; // The linked list is only used for free indices
	Data data;
	data.index = index;
	data.data = _item;
	m_data.push_back(data);
	return makeId(index, indexPtr->generation);
}


template <typename T>
bool Pool<T>::remove(PoolID _id)
{
	if (get(_id) == nullptr)
		return false;

	const u32 index = extractIndexFromId(_id);

	Index& i = m_indices[index];
	Data& data = m_data[i.dataIndex];

	// swap data with the last data of the array, and then remove the last element
	data = m_data.back();
	m_indices[data.index].dataIndex = i.dataIndex;
	m_data.pop_back();

	i.dataIndex = INVALID_INDEX;
	i.next = INVALID_INDEX;
	++i.generation;

	// we put the index back at the end of the free list
	if (m_freeListEnd == INVALID_INDEX)
	{
		YAE_ASSERT(m_freeListBegin == INVALID_INDEX);
		m_freeListBegin = index;
	}
	else
	{
		m_indices[m_freeListEnd].next = index;
	}
	m_freeListEnd = index;
	return true;
}


template <typename T>
void Pool<T>::clear()
{
	m_data.clear();
	for (u32 index = 0; index < m_indices.size(); ++index)
	{
		Index& i = m_indices[index];
		if (i.dataIndex != INVALID_INDEX)
		{
			i.dataIndex = INVALID_INDEX;
			i.next = INVALID_INDEX;
			++i.generation;
			
			if (m_freeListEnd == INVALID_INDEX)
			{
				YAE_ASSERT(m_freeListBegin == INVALID_INDEX);
				m_freeListBegin = index;
			}
			else
			{
				m_indices[m_freeListEnd].next = index;
			}
			m_freeListEnd = index;
		}
	}
}

template <typename T>
u32 Pool<T>::size()
{
	return m_data.size();
}


template <typename T>
T* Pool<T>::begin()
{
	return m_data.begin();
}


template <typename T>
const T* Pool<T>::begin() const
{
	return m_data.begin();
}


template <typename T>
T* Pool<T>::end()
{
	return m_data.end();
}


template <typename T>
const T* Pool<T>::end() const
{
	return m_data.end();
}


template <typename T>
Allocator* Pool<T>::allocator() const
{
	return m_data.allocator();
}

} // namespace yae
