#include <mirror/mirror.h>

#include <yae/ResourceManager.h>

namespace yae {
namespace resource {

template <typename T>
T* find(const char* _name)
{
	ResourceManager& manager = resourceManager();
	YAE_ASSERT_MSG(mirror::GetClass<T>()->isChildOf(mirror::GetClass<Resource>()), "Type is not a child of Resource");
	
	return manager.findResource<T>(_name);
}

template <typename T>
T* findOrCreate(const char* _name)
{
	ResourceManager& manager = resourceManager();
	YAE_ASSERT_MSG(mirror::GetClass<T>()->isChildOf(mirror::GetClass<Resource>()), "Type is not a child of Resource");
	
	T* resource = manager.findResource<T>(_name);
	if (resource == nullptr)
	{
		resource = defaultAllocator().create<T>();
		manager.registerResource(_name, resource);
	}
	return resource;
}

template <typename T>
T* findOrCreateFile(const char* _path)
{
	ResourceManager& manager = resourceManager();
	YAE_ASSERT_MSG(mirror::GetClass<T>()->isChildOf(mirror::GetClass<Resource>()), "Type is not a child of Resource");
	
	T* resource = manager.findResource<T>(_path);
	if (resource == nullptr)
	{
		resource = defaultAllocator().create<T>();
		manager.registerResource(_path, resource);
		resource->setPath(_path);
	}
	return resource;
}

template <typename T>
T* findOrCreateFromFile(const char* _path)
{
	return mirror::Cast<T*>(findOrCreateFromFile(_path));
}

} // namespace resource
} // namespace yae
