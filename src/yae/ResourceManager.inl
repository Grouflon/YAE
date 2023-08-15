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

template <typename T>
const DataArray<Resource*>& ResourceManager::getResourcesByType() const
{
	return getResourcesByType(T::GetClass());
}

} // namespace yae
