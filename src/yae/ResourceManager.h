#pragma once

#include <yae/types.h>
#include <yae/resources/ResourceID.h>
#include <yae/containers/HashMap.h>

#include <mutex>

namespace yae {

class Resource;

class YAE_API ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	void registerResource(const char* _name, Resource* _resource);
	void unregisterResource(Resource* _resource);

	void flushResources();

	Resource* findResource(const char* _name) const;
	Resource* findResource(ResourceID _id) const;
	template <typename T> T* findResource(const char* _name) const;

	const DataArray<Resource*> getResources() const;

	void flagResourceForReload(Resource* _resource);
	void startReloadOnFileChanged(const char* _filePath, Resource* _resource);
	void stopReloadOnFileChanged(const char* _filePath, Resource* _resource);
	void reloadChangedResources();
	void addDependency(Resource* _dependencyResource, Resource* _dependentResource);
	void removeDependency(Resource* _dependencyResource, Resource* _dependentResource);

//private:
	void _processReloadDependencies();

	DataArray<Resource*> m_resources;
	HashMap<StringHash, Resource*> m_resourcesByName;
	HashMap<ResourceID, Resource*> m_resourcesByID;

	HashMap<StringHash, void*> m_fileWatchers;

	std::mutex m_resourcesToReloadMutex;
	DataArray<Resource*> m_resourcesToReload;

	HashMap<Resource*, DataArray<Resource*>> m_dependencies;
};

} // namespace yae

#include "ResourceManager.inl"
