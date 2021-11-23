#pragma once

#include <yae/types.h>
#include <yae/containers/Array.h>

namespace yae {

class Allocator;

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

} // !namespace yae

#include "HashMap.inl"
