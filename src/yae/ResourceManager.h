#pragma once

#include <yae/types.h>
#include <yae/resources/ResourceID.h>
#include <core/containers/HashMap.h>

#include <mutex>

namespace mirror {
class Class;
}

namespace yae {

class Resource;

class YAE_API ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	void gatherResources(const char* _path);

	void registerResource(const char* _name, Resource* _resource);
	void unregisterResource(Resource* _resource);

	void flushResources();

	Resource* findResource(const char* _name) const;
	Resource* findResource(ResourceID _id) const;
	template <typename T> T* findResource(const char* _name) const;

	const DataArray<Resource*>& getResources() const;
	const DataArray<Resource*>& getResourcesByType(const mirror::Class* _class) const;
	template <typename T> const DataArray<Resource*>& getResourcesByType() const;

	void flagResourceForReload(Resource* _resource);
	void registerReloadOnFileChanged(const char* _filePath, Resource* _resource);
	void unregisterReloadOnFileChanged(const char* _filePath, Resource* _resource);
	void reloadChangedResources();
	// TODO: The naming of arguments here is terrible, I need to find something clearer
	void addDependency(Resource* _dependencyResource, Resource* _dependentResource);
	void removeDependency(Resource* _dependencyResource, Resource* _dependentResource);

//private:
	void _processReloadDependencies();

	DataArray<Resource*> m_resources;
	HashMap<StringHash, Resource*> m_resourcesByName;
	HashMap<ResourceID, Resource*> m_resourcesByID;
	mutable HashMap<const mirror::Class*, DataArray<Resource*>> m_resourcesByType;

	HashMap<StringHash, void*> m_fileWatchers;

	std::mutex m_resourcesToReloadMutex;
	DataArray<Resource*> m_resourcesToReload;

	HashMap<Resource*, DataArray<Resource*>> m_dependencies;
};

} // namespace yae

#include "ResourceManager.inl"
