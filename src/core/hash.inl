namespace yae {
namespace hash {

template <typename T>
u32 hash32(const T& _data)
{
	return hash32(&_data, sizeof(T));
}

} // namespace hash
} // namespace yae
