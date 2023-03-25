#pragma once

#include <yae/types.h>

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
	template <typename T> T* findResource(const char* _name) const;

	const DataArray<Resource*> getResources() const;

	void startReloadOnFileChanged(const char* _filePath, Resource* _resource);
	void stopReloadOnFileChanged(const char* _filePath, Resource* _resource);
	void reloadChangedResources();

//private:
	DataArray<Resource*> m_resources;
	HashMap<StringHash, Resource*> m_resourcesByID;

	HashMap<StringHash, void*> m_fileWatchers;

	std::mutex m_resourcesToReloadMutex;
	DataArray<Resource*> m_resourcesToReload;
};

} // namespace yae

#include "ResourceManager.inl"
