#pragma once

namespace yae {

template <typename T>
BaseArray<T>::BaseArray(Allocator* _allocator)
	: m_allocator(_allocator)
{
	if (m_allocator == nullptr)
	{
		m_allocator = &defaultAllocator();
	}
}


template <typename T>
T& BaseArray<T>::operator[](u32 _i)
{
	YAE_ASSERT(_i < m_size);
	return m_data[_i];
}


template <typename T>
const T& BaseArray<T>::operator[](u32 _i) const
{
	YAE_ASSERT(_i < m_size);
	return m_data[_i];
}


template <typename T>
T& BaseArray<T>::front()
{
	YAE_ASSERT(m_size > 0);
	return m_data[0];
}


template <typename T>
const T& BaseArray<T>::front() const
{
	YAE_ASSERT(m_size > 0);
	return m_data[0];
}


template <typename T>
T& BaseArray<T>::back()
{
	YAE_ASSERT(m_size > 0);
	return m_data[m_size - 1];
}


template <typename T>
const T& BaseArray<T>::back() const
{
	YAE_ASSERT(m_size > 0);
	return m_data[m_size - 1];
}


template <typename T>
u32 BaseArray<T>::size() const
{
	return m_size;
}


template <typename T>
u32 BaseArray<T>::capacity() const
{
	return m_capacity;
}


template <typename T>
bool BaseArray<T>::empty() const
{
	return m_size == 0;
}


template <typename T>
T* BaseArray<T>::data() const
{
	return m_data;
}


template <typename T>
T* BaseArray<T>::begin()
{
	return m_data;
}


template <typename T>
const T* BaseArray<T>::begin() const
{
	return m_data;
}


template <typename T>
T* BaseArray<T>::end()
{
	return m_data + m_size;
}


template <typename T>
const T* BaseArray<T>::end() const
{
	return m_data + m_size;
}


template <typename T>
Allocator* BaseArray<T>::allocator() const
{
	return m_allocator;
}

// DataArray

template <typename T>
DataArray<T>::DataArray(Allocator* _allocator)
	: BaseArray(_allocator)
{

}


template <typename T>
DataArray<T>::DataArray(const DataArray<T> &_other, Allocator* _allocator)
	: BaseArray(_allocator)
{
	u32 size = _other.m_size;
	_setCapacity(size);
	m_size = size;
	memcpy(m_data, _other.m_data, m_size * sizeof(T));
}


template <typename T>
DataArray<T>& DataArray<T>::operator=(const DataArray<T>& _other)
{
	u32 size = _other.m_size;
	resize(size);
	memcpy(m_data, _other.m_data, m_size * sizeof(T));
	return *this;
}


template <typename T>
DataArray<T>::~DataArray()
{
	if (m_allocator)
	{
		m_allocator->deallocate(m_data);		
	}
}


template <typename T>
void DataArray<T>::reserve(u32 _newCapacity)
{
	if (_newCapacity > m_capacity)
		_setCapacity(_newCapacity);
}


template <typename T>
void DataArray<T>::resize(u32 _newSize)
{
	if (_newSize > m_capacity)
	{
		_grow(_newSize);
	}

	m_size = _newSize;
}


template <typename T>
void DataArray<T>::clear()
{
	m_size = 0;
}


template <typename T>
T& DataArray<T>::push_back(const T& _item)
{
	resize(m_size + 1);
	T& item = m_data[m_size - 1];
	item = _item;
	return item;
}


template <typename T>
void DataArray<T>::pop_back()
{
	m_size = m_size - 1;
}


template <typename T>
void DataArray<T>::erase(u32 _index, u32 _count)
{
	if (_count == 0)
		return;

	// @OPTIM: May use memcpy here since we are always moving memory backwards, but I'm not sure. This is still undefined behavior according to the c++ doc
	std::memmove(m_data + _index, m_data + _index + _count, (m_size - (_index + _count)) * sizeof(T));
	resize(m_size - _count);
}

template <typename T>
void DataArray<T>::_setCapacity(u32 _newCapacity)
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
void DataArray<T>::_grow(u32 _minCapacity)
{
	u32 newCapacity = m_capacity * 2 + 8;
	if (newCapacity < _minCapacity)
		newCapacity = _minCapacity;

	_setCapacity(newCapacity);
}

// Array

template <typename T>
Array<T>::Array(Allocator* _allocator)
	: BaseArray(_allocator)
{

}


template <typename T>
Array<T>::Array(const Array<T> &_other, Allocator* _allocator)
	: BaseArray(_allocator)
{
	u32 size = _other.m_size;
	_setCapacity(size);
	m_size = size;
	for (u32 i = 0; i < m_size; ++i)
	{
		m_data[i] = _other.m_data[i];
	}
}


template <typename T>
Array<T>& Array<T>::operator=(const Array<T>& _other)
{
	u32 size = _other.m_size;
	resize(size);
	for (u32 i = 0; i < m_size; ++i)
	{
		m_data[i] = _other.m_data[i];
	}
	return *this;
}


template <typename T>
Array<T>::~Array()
{
	clear();
	if (m_allocator)
	{
		m_allocator->deallocate(m_data);		
	}
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
	if (_newSize == m_size)
		return;

	if (_newSize > m_capacity)
	{
		_grow(_newSize);
	}

	if (_newSize > m_size)
	{
		for (u32 i = m_size; i < _newSize; ++i)
		{
			new (m_data + i) T();
		}
	}
	else
	{
		for (u32 i = _newSize; i < m_size; ++i)
		{
			m_data[i].~T();
		}
	}

	m_size = _newSize;
}


template <typename T>
void Array<T>::clear()
{
	for (u32 i = 0; i < m_size; ++i)
	{
		m_data[i].~T();
	}
	m_size = 0;
}


template <typename T>
T& Array<T>::push_back(const T& _item)
{
	resize(m_size + 1);
	new (m_data + m_size - 1) T();
	T& item = m_data[m_size - 1];
	item = _item;
	return item;
}


template <typename T>
void Array<T>::pop_back()
{
	m_data[m_size - 1].~T();
	m_size = m_size - 1;
}


template <typename T>
void Array<T>::erase(u32 _index, u32 _count)
{
	YAE_ASSERT(_index + _count <= m_size);

	if (_count <= 0)
		return;

	u32 rangeEnd = _index + _count;
	for (u32 i = _index; i < rangeEnd; ++i)
	{
		u32 nextValidIndex = i + _count;
		if (nextValidIndex < m_size)
		{
			m_data[i] = m_data[nextValidIndex];
		}
		else
		{
			m_data[i].~T();
		}
	}
	resize(m_size - _count);
}


template <typename T>
void Array<T>::erase(T* _item)
{
	YAE_ASSERT(_item != nullptr);
	YAE_ASSERT(_item >= m_data && _item < (m_data + m_size));
	erase(u32(_item - m_data), 1);
}


template <typename T>
const T* Array<T>::find(const T& _item) const
{
	for (u32 i = 0; i < m_size; ++i)
	{
		if (m_data[i] == _item)
			return m_data + i;
	}
	return nullptr;
}


template <typename T>
T* Array<T>::find(const T& _item)
{
	for (u32 i = 0; i < m_size; ++i)
	{
		if (m_data[i] == _item)
			return m_data + i;
	}
	return nullptr;
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
		for (u32 i = 0; i < m_size; ++i)
		{
			new (newData + i) T();
			newData[i] = m_data[i];
		}
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

} // namespace yae
