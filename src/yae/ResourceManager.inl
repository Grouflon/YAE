#include <mirror/mirror.h>

namespace yae {

template <typename T>
T* ResourceManager::findResource(const char* _name) const
{
	Resource* resource = findResource(_name);
	if (resource == nullptr)
		return nullptr;

	return mirror::Cast<T*>(resource);
}

} // namespace yae
