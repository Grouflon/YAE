#include <core/program.h>

namespace yae {

template<typename Return, typename ...Args>
FunctionPointer<Return, Args...>::FunctionPointer(Return(*_pointer)(Args... _args))
	: pointer(_pointer)
{
	program().registerFunctionPointer((void**)&pointer);
}

template<typename Return, typename ...Args>
FunctionPointer<Return, Args...>::~FunctionPointer()
{
	program().unregisterFunctionPointer((void**)&pointer);
}

template<typename Return, typename ...Args>
Return FunctionPointer<Return, Args...>::operator()(Args... _args)
{
	return pointer(_args...);
}

} // namespace yae
