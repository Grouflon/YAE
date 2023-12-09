#pragma once

namespace yae {

template <typename T>
void BaseArray<T>::resize(u32 _newSize)
{
	if (m_flags & ArrayFlags_CallConstructors)
	{
		resize(_newSize, T());
	}
	else
	{
		if (_newSize > this->m_capacity)
		{
			_grow(_newSize);
		}

		this->m_size = _newSize;
	}
}


template <typename T>
void BaseArray<T>::resize(u32 _newSize, const T& _initValue)
{
	if (_newSize == this->m_size)
		return;

	if (_newSize > this->m_capacity)
	{
		_grow(_newSize);
	}

	if (m_flags & ArrayFlags_CallConstructors)
	{
		if (_newSize > this->m_size)
		{
			for (u32 i = this->m_size; i < _newSize; ++i)
			{
				new (this->m_data + i) T(_initValue);
			}
		}
		else
		{
			for (u32 i = _newSize; i < this->m_size; ++i)
			{
				this->m_data[i].~T();
			}
		}
	}
	else
	{
		if (_newSize > this->m_size)
		{
			for (u32 i = this->m_size; i < _newSize; ++i)
			{
				this->m_data[i] = _initValue;
			}
		}
	}
	
	this->m_size = _newSize;
}


template <typename T>
void BaseArray<T>::clear()
{
	resize(0);
}


template <typename T>
void BaseArray<T>::shrink()
{
	_setCapacity(this->m_size);
}


template <typename T>
T& BaseArray<T>::push_back(const T& _item)
{
	resize(this->m_size + 1);
	if (m_flags & ArrayFlags_CallConstructors)
	{
		new (this->m_data + this->m_size - 1) T();
	}
	T& item = this->m_data[this->m_size - 1];
	item = _item;
	return item;
}


template <typename T>
void BaseArray<T>::push_back(const T* _items, u32 _itemCount)
{
	u32 oldSize = this->m_size;
	resize(this->m_size + _itemCount);
	if (m_flags & ArrayFlags_CallConstructors)
	{
		u32 insertCount = this->m_size - oldSize;
		for (u32 i = 0; i < insertCount; ++i)
		{
			this->m_data[oldSize + i] = _items[i];
		}
	}
	else
	{
		memcpy(this->m_data + oldSize, _items, sizeof(T) * _itemCount);
	}
}


template <typename T>
void BaseArray<T>::pop_back()
{
	if (m_flags & ArrayFlags_CallConstructors)
	{
		this->m_data[this->m_size - 1].~T();
	}
	this->m_size = this->m_size - 1;
}


template <typename T>
void BaseArray<T>::erase(u32 _index, u32 _count)
{
	YAE_ASSERT(_index + _count <= this->m_size);

	if (_count == 0)
		return;

	if (m_flags & ArrayFlags_CallConstructors)
	{
		u32 rangeEnd = _index + _count;
		for (u32 i = _index; i < rangeEnd; ++i)
		{
			u32 nextValidIndex = i + _count;
			if (nextValidIndex < this->m_size)
			{
				this->m_data[i] = this->m_data[nextValidIndex];
			}
			else
			{
				this->m_data[i].~T();
			}
		}
	}
	else
	{
		// @OPTIM: May use memcpy here since we are always moving memory backwards, but I'm not sure. This is still undefined behavior according to the c++ doc
		std::memmove(this->m_data + _index, this->m_data + _index + _count, (this->m_size - (_index + _count)) * sizeof(T));
	}
	
	resize(this->m_size - _count);
}

template <typename T>
void BaseArray<T>::erase(const T& _item)
{
	for (T* it = begin(); it != end(); ++it)
	{
		if (*it == _item)
		{
			erase(it);
			--it;
		}
	}
}

template <typename T>
void BaseArray<T>::erase(bool(*_predicate)(const T&, void*), void* _data)
{
	YAE_ASSERT(_predicate != nullptr);
	for (T* it = begin(); it != end(); ++it)
	{
		if (_predicate(*it, _data))
		{
			erase(it);
			--it;
		}
	}
}

template <typename T>
void BaseArray<T>::erase(T* _item)
{
	YAE_ASSERT(_item != nullptr);
	YAE_ASSERT(_item >= this->m_data && _item < (this->m_data + this->m_size));
	erase(u32(_item - this->m_data), 1);
}


template <typename T>
void BaseArray<T>::reserve(u32 _newCapacity)
{
	if (_newCapacity > this->m_capacity)
		_setCapacity(_newCapacity);
}


template <typename T>
void BaseArray<T>::swap(T* _iteratorA, T* _iteratorB)
{
	YAE_ASSERT(_iteratorA != nullptr);
	YAE_ASSERT(_iteratorA >= this->m_data && _iteratorA < (this->m_data + this->m_size));
	YAE_ASSERT(_iteratorB != nullptr);
	YAE_ASSERT(_iteratorB >= this->m_data && _iteratorB < (this->m_data + this->m_size));

	T temp = *_iteratorB;
	*_iteratorB = *_iteratorA;
	*_iteratorA = temp;
}


template <typename T>
void BaseArray<T>::swap(u32 _indexA, u32 _indexB)
{
	swap(begin() + _indexA, begin() + _indexB);
}


template <typename T>
BaseArray<T>& BaseArray<T>::operator=(const BaseArray<T>& _other)
{
	u32 size = _other.m_size;
	resize(size);
	if (m_flags & ArrayFlags_CallConstructors)
	{
		for (u32 i = 0; i < this->m_size; ++i)
		{
			this->m_data[i] = _other.m_data[i];
		}
	}
	else
	{
		memcpy(this->m_data, _other.m_data, this->m_size * sizeof(T));
	}
	return *this;
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
const T* BaseArray<T>::find(const T& _item) const
{
	for (const T* it = begin(); it != end(); ++it)
	{
		if (*it == _item)
			return it;
	}
	return nullptr;
}


template <typename T>
T* BaseArray<T>::find(const T& _item)
{
	for (T* it = begin(); it != end(); ++it)
	{
		if (*it == _item)
			return it;
	}
	return nullptr;
}


template <typename T>
const T* BaseArray<T>::find(bool (*_predicate)(const T&, void*), void* _data) const
{
	for (T* it = begin(); it != end(); ++it)
	{
		if (_predicate(*it, _data))
			return it;
	}
	return nullptr;
}


template <typename T>
T* BaseArray<T>::find(bool (*_predicate)(const T&, void*), void* _data)
{
	for (T* it = begin(); it != end(); ++it)
	{
		if (_predicate(*it, _data))
			return it;
	}
	return nullptr;
}


template <typename T>
bool BaseArray<T>::operator==(const BaseArray<T>& _rhs) const
{
	if (m_size != _rhs.m_size)
		return false;

	for (u32 i = 0; i < m_size; ++i)
	{
		if ((*this)[i] != _rhs[i])
			return false;
	}
	return true;
}


template <typename T>
bool BaseArray<T>::operator!=(const BaseArray<T>& _rhs) const
{
	return !(*this == _rhs);
	if (m_size != _rhs.m_size)
		return false;

	for (u32 i = 0; i < m_size; ++i)
	{
		if ((*this)[i] != _rhs[i])
			return false;
	}
	return true;
}


template <typename T>
Allocator* BaseArray<T>::allocator() const
{
	return m_allocator;
}


template <typename T>
BaseArray<T>::BaseArray(Allocator* _allocator, ArrayFlags _flags)
	: m_allocator(_allocator)
	, m_flags(_flags)
{
	if (m_allocator == nullptr)
	{
		m_allocator = &defaultAllocator();
	}
}


template <typename T>
BaseArray<T>::~BaseArray()
{
	clear();
	if (this->m_allocator)
	{
		this->m_allocator->deallocate(this->m_data);		
	}
}


template <typename T>
void BaseArray<T>::_setCapacity(u32 _newCapacity)
{
	if (this->m_capacity == _newCapacity)
			return;

	if (_newCapacity < this->m_capacity)
		resize(_newCapacity);

	T* newData = nullptr;
	if (_newCapacity > 0)
	{
		newData = (T*)this->m_allocator->allocate(sizeof(T) * _newCapacity, alignof(T));

		if (m_flags & ArrayFlags_CallConstructors)
		{
			for (u32 i = 0; i < this->m_size; ++i)
			{
				new (newData + i) T();
				newData[i] = this->m_data[i];
				this->m_data[i].~T();
			}
		}
		else
		{
			memcpy(newData, this->m_data, sizeof(T) * this->m_size);
		}
		
	}
	this->m_allocator->deallocate(this->m_data);
	this->m_data = newData;
	this->m_capacity = _newCapacity;
}


template <typename T>
void BaseArray<T>::_grow(u32 _minCapacity)
{
	u32 newCapacity = this->m_capacity * 2 + 8;
	if (newCapacity < _minCapacity)
		newCapacity = _minCapacity;

	_setCapacity(newCapacity);
}


// DataArray
template <typename T>
DataArray<T>::DataArray(Allocator* _allocator)
	: BaseArray<T>(_allocator, 0)
{

}


template <typename T>
DataArray<T>::DataArray(const DataArray<T> &_other, Allocator* _allocator)
	: BaseArray<T>(_allocator, 0)
{
	*this = _other;
}

// Array
template <typename T>
Array<T>::Array(Allocator* _allocator)
	: BaseArray<T>(_allocator, ArrayFlags_CallConstructors)
{

}


template <typename T>
Array<T>::Array(const Array<T> &_other, Allocator* _allocator)
	: BaseArray<T>(_allocator, ArrayFlags_CallConstructors)
{
	*this = _other;
}

} // namespace yae
