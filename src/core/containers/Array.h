#pragma once

#include <core/types.h>
#include <core/memory.h>

#include <cstring>
#include <type_traits>
#include <new>

namespace yae {

class Allocator;

typedef u8 ArrayFlags;
enum ArrayFlags_
{
	ArrayFlags_CallConstructors = 1 << 0,
};


template <typename T>
class BaseArray
{
public:
	// Edit
	void resize(u32 _newSize);
	void resize(u32 _newSize, const T& _initValue);
	void clear();
	void shrink();
	T& push_back(const T& _item);
	void push_back(const T* _items, u32 _itemCount);
	void pop_back();
	void erase(u32 _index, u32 _count);
	void erase(T* _item);
	void erase(const T& _item);
	void erase(bool(*_predicate)(const T&, void*), void* _data = nullptr);
	void reserve(u32 _newCapacity);
	void swap(T* _iteratorA, T* _iteratorB);
	void swap(u32 _indexA, u32 _indexB);
	BaseArray<T>& operator=(const BaseArray<T>& _other);

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

	// Find
	const T* find(const T& _item) const;
	T* find(const T& _item);
	const T* find(bool (*_predicate)(const T&, void*), void* _data = nullptr) const;
	T* find(bool (*_predicate)(const T&, void*), void* _data = nullptr);

	// Comparison
	bool operator==(const BaseArray<T>& _rhs) const;
	bool operator!=(const BaseArray<T>& _rhs) const;

	// Misc
	Allocator* allocator() const;

protected:
	BaseArray(Allocator* _allocator, ArrayFlags _flags);
	~BaseArray();
	// @TODO: move constructor ?

private:
	void _setCapacity(u32 _newCapacity);
	void _grow(u32 _minCapacity);

	Allocator* m_allocator = nullptr;
	u32 m_size = 0;
	u32 m_capacity = 0;
	T* m_data = nullptr;
	const u8 m_flags = 0;
};


template <typename T>
class DataArray : public BaseArray<T>
{
	static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

public:
	DataArray(Allocator* _allocator = nullptr);
	DataArray(const DataArray<T> &_other, Allocator* _allocator = nullptr);
	DataArray<T>& operator=(const DataArray<T> &_other) = default;
};


template <typename T>
class Array : public BaseArray<T>
{
	static_assert(std::is_default_constructible<T>::value, "T must be default constructible");

public:
	Array(Allocator* _allocator = nullptr);
	Array(const Array<T> &_other, Allocator* _allocator = nullptr);
	Array<T>& operator=(const Array<T> &_other) = default;
};

} // namespace yae

#include "Array.inl"
