#include "ResourceManager.h"

#include <yae/string.h>
#include <yae/resource.h>
#include <yae/resources/Resource.h>

#include <FileWatch/FileWatch.hpp>

namespace yae {


ResourceManager::ResourceManager()
	: m_resources(&defaultAllocator())
	, m_resourcesByID(&defaultAllocator())
{
}

ResourceManager::~ResourceManager()
{
	flushResources();

	YAE_ASSERT_MSG(m_resources.size() == 0, "Resources list must be empty when the manager gets destroyed");
}

void ResourceManager::registerResource(const char* _name, Resource* _resource)
{
	YAE_ASSERT(_resource != nullptr);
	YAE_ASSERT(std::find(m_resources.begin(), m_resources.end(), _resource) == m_resources.end());

	YAE_VERIFY(string::safeCopyToBuffer(_resource->m_name, _name, countof(_resource->m_name)) < countof(_resource->m_name));
	StringHash id = StringHash(_resource->m_name);	
	YAE_ASSERT(m_resourcesByID.get(id) == nullptr);

	m_resources.push_back(_resource);
	m_resourcesByID.set(id, _resource);

	YAE_VERBOSEF_CAT("resource", "Registered \"%s\"...", _resource->m_name);
}

void ResourceManager::unregisterResource(Resource* _resource)
{
	YAE_ASSERT(_resource != nullptr);

	{
		auto it = std::find(m_resources.begin(), m_resources.end(), _resource);
		YAE_ASSERT(it != m_resources.end());
		m_resources.erase(it);
	}

	{
		StringHash id = StringHash(_resource->m_name);	
		YAE_ASSERT(m_resourcesByID.get(id) != nullptr);
		m_resourcesByID.remove(id);
	}

	YAE_VERBOSEF_CAT("resource", "Unregistered \"%s\"...", _resource->m_name);
	_resource->m_name[0] = 0;
}

Resource* ResourceManager::findResource(const char* _name) const
{
	StringHash id = StringHash(_name);
	Resource*const* resourcePtr = m_resourcesByID.get(id);
	if (resourcePtr == nullptr)
		return nullptr;

	return *resourcePtr;
}

void ResourceManager::flushResources()
{
	// Gather unused resources
	DataArray<Resource*> toDeleteResources(&scratchAllocator());
	for (Resource* resource : m_resources)
	{
		if (!resource->isLoaded())
		{
			toDeleteResources.push_back(resource);
		}
	}

	// Unregister & delete
	for (Resource* resource : toDeleteResources)
	{
		unregisterResource(resource);
		defaultAllocator().destroy(resource);
	}
	toDeleteResources.clear();
}

const DataArray<Resource*> ResourceManager::getResources() const
{
	return m_resources;
}

void ResourceManager::startReloadOnFileChanged(const char* _filePath, Resource* _resource)
{
	StringHash id(_filePath);

	YAE_ASSERT(m_fileWatchers.get(id) == nullptr);

	ResourceManager* rm = &resourceManager();
	auto fileWatch = defaultAllocator().create<filewatch::FileWatch<std::string>>(
		_filePath,
		[_resource, rm](const std::string& _path, const filewatch::Event _change_type)
		{
			if (_change_type == filewatch::Event::modified)
			{
				rm->m_resourcesToReloadMutex.lock();
				rm->m_resourcesToReload.push_back(_resource);
				rm->m_resourcesToReloadMutex.unlock();
				YAE_VERBOSEF_CAT("resource", "\"%s\" modified.", _path.c_str());
			}
		}
	);
	m_fileWatchers.set(id, fileWatch);
}

void ResourceManager::stopReloadOnFileChanged(const char* _filePath, Resource* _resource)
{
	StringHash id(_filePath);

	auto* watcherPtr = (filewatch::FileWatch<std::string>**)m_fileWatchers.get(id);
	YAE_ASSERT(watcherPtr != nullptr);

	defaultAllocator().destroy(*watcherPtr);
	m_fileWatchers.remove(id);
}

void ResourceManager::reloadChangedResources()
{
	m_resourcesToReloadMutex.lock();
	_processReloadDependencies();
	DataArray<Resource*> resourcesToReload(m_resourcesToReload, &scratchAllocator());
	m_resourcesToReload.clear();
	m_resourcesToReloadMutex.unlock();
	
	for (Resource* resource : resourcesToReload)
	{
		resource->reload();
	}
}

void ResourceManager::addDependency(Resource* _dependencyResource, Resource* _dependentResource)
{
	YAE_ASSERT(_dependencyResource != nullptr);
	YAE_ASSERT(_dependentResource != nullptr);

	DataArray<Resource*>* dependentResourcesPtr = m_dependencies.get(_dependencyResource);
	if (dependentResourcesPtr == nullptr)
	{
		dependentResourcesPtr = &m_dependencies.set(_dependencyResource, DataArray<Resource*>());
	}
	YAE_ASSERT(dependentResourcesPtr != nullptr);
	dependentResourcesPtr->push_back(_dependentResource);
}

void ResourceManager::removeDependency(Resource* _dependencyResource, Resource* _dependentResource)
{
	YAE_ASSERT(_dependencyResource != nullptr);
	YAE_ASSERT(_dependentResource != nullptr);

	DataArray<Resource*>* dependentResourcesPtr = m_dependencies.get(_dependencyResource);
	YAE_ASSERT(dependentResourcesPtr != nullptr);
	auto it = dependentResourcesPtr->find(_dependentResource);
	YAE_ASSERT(it != dependentResourcesPtr->end());
	dependentResourcesPtr->erase(it);
}

void ResourceManager::_processReloadDependencies()
{
	for (u32 i = 0; i < m_resourcesToReload.size(); ++i)
	{
		Resource* resource = m_resourcesToReload[i];
		
		DataArray<Resource*>* dependentResourcesPtr = m_dependencies.get(resource);
		if (dependentResourcesPtr == nullptr)
			continue;
		
		for (Resource* dependentResource : *dependentResourcesPtr)
		{
			if (m_resourcesToReload.find(dependentResource) == nullptr)
			{
				m_resourcesToReload.push_back(dependentResource);
			}
		}
	}
}

} // namespace yae
