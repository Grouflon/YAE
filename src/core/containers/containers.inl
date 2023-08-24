
namespace yae {
namespace containers {

template <typename T>
const T* find(const BaseArray<T>& _array, const T& _value)
{
	for (const T& value : _array)
	{
		if (value == _value)
			return &value;
	}
	return nullptr;
}

template <typename T>
T* find(BaseArray<T>& _array, const T& _value)
{
	for (T& value : _array)
	{
		if (value == _value)
			return &value;
	}
	return nullptr;
}

template <typename T>
u32 remove(Array<T>& _array, const T& _value)
{
	u32 removedCount = 0;
	for (u32 i = 0; i < _array.size();)
	{
		if (_array[i] == _value)
		{
			_array.erase(i, 1);
			++removedCount;
		}
		else
		{
			++i;
		}
	}
	return removedCount;
}

template <typename T>
u32 remove(DataArray<T>& _array, const T& _value)
{
	u32 removedCount = 0;
	for (u32 i = 0; i < _array.size();)
	{
		if (_array[i] == _value)
		{
			_array.erase(i, 1);
			++removedCount;
		}
		else
		{
			++i;
		}
	}
	return removedCount;
}


} // namespace containers
} // namespace yae
