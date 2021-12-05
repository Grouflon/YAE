#pragma once

#include <yae/types.h>
#include <yae/memory.h>

#include <cstring>
#include <type_traits>
#include <new>

namespace yae {

class Allocator;

template <typename T>
class BaseArray
{
public:
	BaseArray(Allocator* _allocator = nullptr);
	// @TODO: move constructor ?

	// Accessors
	T& operator[](u32 _i);
	const T& operator[](u32 _i) const;
	T& front();
	const T& front() const;
	T& back();
	const T& back() const;
	u32 size() const;
	u32 capacity() const;
	bool empty() const;
	T* data() const;

	// Iterators
	T* begin();
	const T* begin() const;
	T* end();
	const T* end() const;

	// Misc
	Allocator* allocator() const;

protected:

	Allocator* m_allocator = nullptr;
	u32 m_size = 0;
	u32 m_capacity = 0;
	T* m_data = nullptr;
};


template <typename T>
class DataArray : public BaseArray<T>
{
	static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

public:
	DataArray(Allocator* _allocator = nullptr);
	DataArray(const DataArray<T> &_other, Allocator* _allocator = nullptr);
	DataArray<T>& operator=(const DataArray<T>& _other);
	~DataArray();

	void resize(u32 _newSize);
	void clear();
	T& push_back(const T& _item);
	void pop_back();
	void erase(u32 _index, u32 _count);
	void reserve(u32 _newCapacity);

protected:
	void _setCapacity(u32 _newCapacity);
	void _grow(u32 _minCapacity);
};


template <typename T>
class Array : public BaseArray<T>
{
	static_assert(std::is_default_constructible<T>::value, "T must be default constructible");

public:
	Array(Allocator* _allocator = nullptr);
	Array(const Array<T> &_other, Allocator* _allocator = nullptr);
	Array<T>& operator=(const Array<T>& _other);
	~Array();

	void resize(u32 _newSize);
	void clear();
	T& push_back(const T& _item);
	void pop_back();
	void erase(u32 _index, u32 _count);
	void erase(T* _item);
	void reserve(u32 _newCapacity);

	const T* find(const T& _item) const;
	T* find(const T& _item);

protected:
	void _setCapacity(u32 _newCapacity);
	void _grow(u32 _minCapacity);
};

} // !namespace yae

#include "Array.inl"
