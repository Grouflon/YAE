#pragma once

namespace yae {

template<typename Key, typename T>
HashMap<Key, T>::HashMap(Allocator* _allocator)
	: m_hash(_allocator)
	, m_data(_allocator)
{
}


template<typename Key, typename T>
bool HashMap<Key, T>::has(Key _key) const
{
	FindResult result = _find(_key);
	return result.dataIndex != END_OF_LIST;
}


template<typename Key, typename T>
const T* HashMap<Key, T>::get(Key _key) const
{
	FindResult result = _find(_key);
	return result.dataIndex != END_OF_LIST ? &m_data[result.dataIndex].value : nullptr;
}


template<typename Key, typename T>
T* HashMap<Key, T>::get(Key _key)
{
	FindResult result = _find(_key);
	return result.dataIndex != END_OF_LIST ? &m_data[result.dataIndex].value : nullptr;
}


template<typename Key, typename T>
T& HashMap<Key, T>::set(Key _key, const T& _value)
{
	if (m_hash.size() == 0)
	{
		_grow();
	}

	const u32 i = _find_or_make(_key);
	m_data[i].value = _value;

	if (_isFull())
	{
		_grow();
	}

	return m_data[i].value;
}


template<typename Key, typename T>
void HashMap<Key, T>::remove(Key _key)
{
	FindResult result = _find(_key);
	if (result.dataIndex != END_OF_LIST)
	{
		_erase(result);
	}
}


template<typename Key, typename T>
u32 HashMap<Key, T>::size() const
{
	return m_data.size();
}


template<typename Key, typename T>
bool HashMap<Key, T>::empty() const
{
	return m_data.size() == 0;
}


template<typename Key, typename T>
void HashMap<Key, T>::reserve(u32 _size)
{
	if (_size > m_hash.size())
	{
		_rehash(_size);
	}
}


template<typename Key, typename T>
void HashMap<Key, T>::clear()
{
	m_hash.clear();
	m_data.clear();
}


template<typename Key, typename T>
void HashMap<Key, T>::shrink()
{
	m_hash.shrink();
	m_data.shrink();
}



template<typename Key, typename T>
const typename HashMap<Key, T>::Entry* HashMap<Key, T>::begin() const
{
	return m_data.begin();
}


template<typename Key, typename T>
const typename HashMap<Key, T>::Entry* HashMap<Key, T>::end() const
{
	return m_data.end();
}


template<typename Key, typename T>
typename HashMap<Key, T>::Entry* HashMap<Key, T>::begin()
{
	return m_data.begin();
}


template<typename Key, typename T>
typename HashMap<Key, T>::Entry* HashMap<Key, T>::end()
{
	return m_data.end();
}

template <typename Key, typename T>
u32 HashMap<Key, T>::_addEntry(Key _key)
{
	Entry entry;
	entry.key = _key;
	entry.next = END_OF_LIST;
	u32 entryIndex = m_data.size();
	m_data.push_back(entry);
	return entryIndex;
}


template <typename Key, typename T>
u32 HashMap<Key, T>::_make(Key _key)
{
	const FindResult result = _find(_key);
	const u32 i = _addEntry(_key);

	if (result.previousDataIndex == END_OF_LIST)
	{
		m_hash[result.hashIndex] = i;
	}
	else
	{
		m_data[result.previousDataIndex].next = i;
	}
	m_data[i].next = result.dataIndex;

	return i;
}


template<typename Key, typename T>
void HashMap<Key, T>::_insert(Key _key, const T& _value)
{
	if (m_hash.size() == 0)
	{
		_grow();
	}

	const u32 i = _make(_key);
	m_data[i].value = _value;
	if (_isFull())
	{
		_grow();
	}
}


template<typename Key, typename T>
void HashMap<Key, T>::_erase(const FindResult& _result)
{
	YAE_ASSERT(_result.dataIndex != END_OF_LIST);

	if (_result.previousDataIndex == END_OF_LIST)
	{
		m_hash[_result.hashIndex] = m_data[_result.dataIndex].next;
	}
	else
	{
		m_data[_result.previousDataIndex].next = m_data[_result.dataIndex].next;
	}

	// early exit
	if (_result.dataIndex == m_data.size() - 1)
	{
		m_data.pop_back();
		return;
	}
		
	m_data[_result.dataIndex] = m_data[m_data.size() - 1];
	FindResult last = _find(m_data[_result.dataIndex].key);

	if (last.previousDataIndex == END_OF_LIST)
	{
		m_hash[last.hashIndex] = _result.dataIndex;
	}
	else
	{
		m_data[last.previousDataIndex].next = _result.dataIndex;
	}
	m_data.pop_back();
}


template <typename Key, typename T>
typename HashMap<Key, T>::FindResult HashMap<Key, T>::_find(Key _key) const
{
	FindResult result;
	result.hashIndex = END_OF_LIST;
	result.previousDataIndex = END_OF_LIST;
	result.dataIndex = END_OF_LIST;

	u64 hashSize = u64(m_hash.size());
	if (hashSize == 0)
		return result;

	result.hashIndex = size_t(_key) % hashSize;
	result.dataIndex = m_hash[result.hashIndex];
	while (result.dataIndex != END_OF_LIST)
	{
		if (m_data[result.dataIndex].key == _key)
			return result;

		result.previousDataIndex = result.dataIndex;
		result.dataIndex = m_data[result.dataIndex].next;
	}
	return result;
}


template <typename Key, typename T>
u32 HashMap<Key, T>::_find_or_make(Key _key)
{
	const FindResult result = _find(_key);
	if (result.dataIndex != END_OF_LIST)
		return result.dataIndex;

	u32 i = _addEntry(_key);
	if (result.previousDataIndex == END_OF_LIST)
	{
		m_hash[result.hashIndex] = i;
	}
	else
	{
		m_data[result.previousDataIndex].next = i;
	}
	return i;
}


template <typename Key, typename T>
bool HashMap<Key, T>::_isFull() const
{
	const float max_load_factor = 0.7f;
	return m_data.size() >= m_hash.size() * max_load_factor;
}


template <typename Key, typename T>
void HashMap<Key, T>::_grow()
{
	u32 newSize = m_data.size() * 2 + 10;
	_rehash(newSize);
}


template <typename Key, typename T>
void HashMap<Key, T>::_rehash(u32 _newSize)
{
	HashMap<Key, T> newHashMap(m_hash.allocator());
	newHashMap.m_hash.resize(_newSize);
	newHashMap.m_data.reserve(m_data.size());

	for (u32 i = 0; i < _newSize; ++i)
	{
		newHashMap.m_hash[i] = END_OF_LIST;
	}

	for (u32 i = 0; i < m_data.size(); ++i)
	{
		const Entry& entry = m_data[i];
		newHashMap._insert(entry.key, entry.value);
	}

	HashMap<Key, T> emptyHashMap(m_hash.allocator());
	this->~HashMap<Key, T>();
	memcpy(this, &newHashMap, sizeof(*this));
	memcpy(&newHashMap, &emptyHashMap, sizeof(*this));
}

} // namespace yae
