#include <core/hash.h>

namespace yae {

template <typename ...Args>
void Event<Args...>::bind(DelegateFunction _delegate)
{
	YAE_ASSERT(_delegate != nullptr);
	u32 hash = _hashFunctionPointer(_delegate);

	m_delegates.set(hash, _delegate);
}

template <typename ...Args>
void Event<Args...>::unbind(DelegateFunction _delegate)
{
	YAE_ASSERT(_delegate != nullptr);
	u32 hash = _hashFunctionPointer(_delegate);

	YAE_ASSERT(m_delegates.get(hash) != nullptr);

	m_delegates.remove(hash);
}

template <typename ...Args>
template <typename T>
void Event<Args...>::bind(T* _self, typename Event<Args...>::DelegateMethod<T> _delegate)
{
	YAE_ASSERT(_delegate != nullptr);
	u32 hash = _hashMethodPointer(_self, _delegate);

	std::function<void(Args...)> f = [_self, _delegate](Args... _args)
	{
		(_self->*_delegate)(_args...);
	};

	m_delegates.set(hash, f);
}

template <typename ...Args>
template <typename T>
void Event<Args...>::unbind(T* _self, typename Event<Args...>::DelegateMethod<T> _delegate)
{
	YAE_ASSERT(_delegate != nullptr);
	u32 hash = _hashMethodPointer(_self, _delegate);

	YAE_ASSERT(m_delegates.get(hash) != nullptr);

	m_delegates.remove(hash);
}

template <typename ...Args>
void Event<Args...>::dispatch(Args... _args)
{
	for (auto& f : m_delegates)
	{
		f.value(_args...);
	}
}

template <typename ...Args>
u32 Event<Args...>::_hashFunctionPointer(DelegateFunction _delegate) const
{
	return hash::hash32(_delegate);
}

template <typename ...Args>
template <typename T>
u32 Event<Args...>::_hashMethodPointer(T* _self, DelegateMethod<T> _delegate) const
{
	struct PointerAndMethod
	{
		T* self;
		DelegateMethod<T> method;
	};
	PointerAndMethod pointerAndMethod;
	pointerAndMethod.self = _self;
	pointerAndMethod.method = _delegate;

	return hash::hash32(pointerAndMethod);
}

} // namespace yae
