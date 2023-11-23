#pragma once

#include <core/types.h>

namespace yae {

template<typename Return, typename ...Args>
struct FunctionPointer
{
	FunctionPointer(Return(*_pointer)(Args... _args) = nullptr);
	~FunctionPointer();

	Return(*pointer)(Args...);

	Return operator()(Args...);
};

} // namespace yae

#include "FunctionPointer.inl"
