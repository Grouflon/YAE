#pragma once

#include <types.h>
#include <memory.h>

#include <cstring>

namespace yae {

class Allocator;

template <typename T>
class Array
{
public:
	Array(Allocator* _allocator = nullptr);
	Array(const Array<T> &_other);
	Array<T>& operator=(const Array<T>& _other);
	~Array();
	// @TODO: move constructor ?

	// Accessors
	T& operator[](u32 _i);
	const T& operator[](u32 _i) const;
	T& front();
	const T& front() const;
	T& back();
	const T& back() const;
	u32 size() const;
	bool empty() const;
	T* data() const;

	// Iterators
	T* begin();
	const T* begin() const;
	T* end();
	const T* end() const;

	// Manipulation
	void reserve(u32 _newCapacity);
	void resize(u32 _newSize);
	void clear();
	void push_back(const T& _item);
	void pop_back();
	void erase(u32 _index, u32 _count);

	// Misc
	Allocator* allocator() const;

private:
	void _setCapacity(u32 _newCapacity);
	void _grow(u32 _minCapacity);

	Allocator* m_allocator = nullptr;
	u32 m_size = 0;
	u32 m_capacity = 0;
	T* m_data = nullptr;
};


template <typename Key, typename T>
class HashMap
{
	static_assert(std::is_default_constructible<Key>::value, "Key must be default constructible");
	static_assert(std::is_default_constructible<T>::value, "T must be default constructible");

public:
	struct Entry
	{
		Key key;
		u32 next;
		T value;
	};

	HashMap(Allocator* _allocator);

	bool has(Key _key) const;
	const T* get(Key _key) const;
	T* get(Key _key);
	T& set(Key _key, const T& _value);
	void remove(Key _key);
	u32 size() const;
	bool empty() const;

	void reserve(u32 _size);
	void clear();

	const Entry* begin() const;
	const Entry* end() const;
	Entry* begin();
	Entry* end();

private:
	static const u32 END_OF_LIST = 0xffffffffu;

	struct FindResult
	{
		u32 hashIndex;
		u32 previousDataIndex;
		u32 dataIndex;
	};

	u32 _addEntry(Key _key);
	u32 _make(Key _key);
	void _insert(Key _key, const T& _value);
	void _erase(const FindResult& _result);

	FindResult _find(Key _key) const;
	u32 _find_or_make(Key _key);

	bool _isFull() const;
	void _grow();
	void _rehash(u32 _newSize);

	Array<u32> m_hash;
	Array<Entry> m_data;
};


// Definitions

// Array
template <typename T>
Array<T>::Array(Allocator* _allocator)
	: m_allocator(_allocator)
{
}


template <typename T>
Array<T>::Array(const Array<T> &_other)
	: Array(_other.m_allocator)
{
	u32 size = _other.m_size;
	_setCapacity(size);
	memcpy(m_data, _other.m_data, size);
	m_size = size;
}


template <typename T>
Array<T>& Array<T>::operator=(const Array<T>& _other)
{
	if (m_allocator != _other.m_allocator)
	{
		this->~Array();
		m_allocator = _other.m_allocator;
	}
	u32 size = _other.m_size;
	resize(size);
	memcpy(m_data, _other.m_data, size);
	return *this;
}


template <typename T>
Array<T>::~Array()
{
	if (m_allocator)
	{
		m_allocator->deallocate(m_data);		
	}
}


template <typename T>
T& Array<T>::operator[](u32 _i)
{
	YAE_ASSERT(_i < m_size);
	return m_data[_i];
}


template <typename T>
const T& Array<T>::operator[](u32 _i) const
{
	YAE_ASSERT(_i < m_size);
	return m_data[_i];
}


template <typename T>
T& Array<T>::front()
{
	YAE_ASSERT(m_size > 0);
	return m_data[0];
}


template <typename T>
const T& Array<T>::front() const
{
	YAE_ASSERT(m_size > 0);
	return m_data[0];
}


template <typename T>
T& Array<T>::back()
{
	YAE_ASSERT(m_size > 0);
	return m_data[m_size - 1];
}


template <typename T>
const T& Array<T>::back() const
{
	YAE_ASSERT(m_size > 0);
	return m_data[m_size - 1];
}


template <typename T>
u32 Array<T>::size() const
{
	return m_size;
}


template <typename T>
bool Array<T>::empty() const
{
	return m_size == 0;
}


template <typename T>
T* Array<T>::data() const
{
	return m_data;
}


template <typename T>
T* Array<T>::begin()
{
	return m_data;
}


template <typename T>
const T* Array<T>::begin() const
{
	return m_data;
}


template <typename T>
T* Array<T>::end()
{
	return m_data + m_size;
}


template <typename T>
const T* Array<T>::end() const
{
	return m_data + m_size;
}


template <typename T>
void Array<T>::reserve(u32 _newCapacity)
{
	if (_newCapacity > m_capacity)
		_setCapacity(_newCapacity);
}


template <typename T>
void Array<T>::resize(u32 _newSize)
{
	if (_newSize > m_capacity)
	{
		_grow(_newSize);
	}
	m_size = _newSize;
}


template <typename T>
void Array<T>::clear()
{
	m_size = 0;
}


template <typename T>
void Array<T>::push_back(const T& _item)
{
	resize(m_size + 1);
	m_data[m_size - 1] = _item;
}


template <typename T>
void Array<T>::pop_back()
{
	--m_size;
}


template <typename T>
void Array<T>::erase(u32 _index, u32 _count)
{
	if (_count == 0)
		return;

	// @OPTIM: May use memcpy here since we are always moving memory backwards, but I'm not sure. This is still undefined behavior according to the c++ doc
	std::memmove(m_data + _index, m_data + _index + _count, (m_size - (_index + _count)) * sizeof(T));
	m_size -= _count;
}


template <typename T>
Allocator* Array<T>::allocator() const
{
	return m_allocator;
}


template <typename T>
void Array<T>::_setCapacity(u32 _newCapacity)
{
	if (m_capacity == _newCapacity)
		return;

	if (_newCapacity < m_capacity)
		resize(_newCapacity);

	T* newData = nullptr;
	if (_newCapacity > 0)
	{
		newData = (T*)m_allocator->allocate(sizeof(T) * _newCapacity, alignof(T));
		memcpy(newData, m_data, sizeof(T) * m_size);
	}
	m_allocator->deallocate(m_data);
	m_data = newData;
	m_capacity = _newCapacity;
}


template <typename T>
void Array<T>::_grow(u32 _minCapacity)
{
	u32 newCapacity = m_capacity * 2 + 8;
	if (newCapacity < _minCapacity)
		newCapacity = _minCapacity;

	_setCapacity(newCapacity);
}

// HashMap

template<typename Key, typename T>
HashMap<Key, T>::HashMap(Allocator* _allocator)
	: m_hash(_allocator)
	, m_data(_allocator)
{
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
typename const HashMap<Key, T>::Entry* HashMap<Key, T>::begin() const
{
	return m_data.begin();
}


template<typename Key, typename T>
typename const HashMap<Key, T>::Entry* HashMap<Key, T>::end() const
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

	u32 hashSize = m_hash.size();
	if (hashSize == 0)
		return result;

	result.hashIndex = _key % hashSize;
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