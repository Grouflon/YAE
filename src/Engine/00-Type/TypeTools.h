#pragma once

namespace yae {

template <typename T, size_t N>
constexpr size_t countof(const T(&_array)[N])
{
	return N;
}

}