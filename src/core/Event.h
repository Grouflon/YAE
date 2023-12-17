#pragma once

#include <core/types.h>
#include <core/containers/Array.h>

#include <functional>

namespace yae {

template <typename ...Args>
struct Event
{
	typedef void (*DelegateFunction)(Args...);
	template <typename T>
	using DelegateMethod = void(T::*)(Args...);

	void bind(DelegateFunction _delegate);
	void unbind(DelegateFunction _delegate);

	template <typename T>
	void bind(T* _self, DelegateMethod<T> _delegate);
	template <typename T>
	void unbind(T* _self, DelegateMethod<T> _delegate);
	
	void dispatch(Args... _args);

//private:
	u32 _hashFunctionPointer(DelegateFunction _delegate) const;
	template <typename T>
	u32 _hashMethodPointer(T* _self, DelegateMethod<T> _delegate) const;

	HashMap<u32, std::function<void(Args...)>> m_delegates;
};

} // namespace yae

#include "Event.inl"
