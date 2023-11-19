#include "ResourceManager.h"

#include <core/string.h>
#include <core/time.h>
#include <core/hash.h>
#include <core/filesystem.h>

#include <yae/resources/Resource.h>
#include <yae/resource.h>
#include <yae/Engine.h>
#include <yae/FileWatchSystem.h>

namespace yae {


ResourceManager::ResourceManager()
	: m_resources(&defaultAllocator())
	, m_resourcesByName(&defaultAllocator())
	, m_resourcesByID(&defaultAllocator())
{
}

ResourceManager::~ResourceManager()
{
	flushResources();

	YAE_ASSERT_MSG(m_resources.size() == 0, "Resources list must be empty when the manager gets destroyed");
}

void ResourceManager::gatherResources(const char* _path)
{
	String path = String(filesystem::normalizePath(_path), &scratchAllocator());

	YAE_VERBOSEF_CAT("resource", "Gathering resources inside \"%s\"...", path.c_str());

	filesystem::walkDirectory(path.c_str(), [](const filesystem::Entry& _entry, void* _userData)
	{
		String extension(filesystem::getExtension(_entry.path.c_str()), &scratchAllocator());
		if (strcmp(extension.c_str(), "res") == 0)
		{
			resource::findOrCreateFromFile(_entry.path.c_str());
		}
		return true;
	}
	, true, filesystem::EntryType_File);

	YAE_VERBOSEF_CAT("resource", "Gathering done.");
}

void ResourceManager::registerResource(const char* _name, Resource* _resource)
{
	YAE_ASSERT(_resource != nullptr);
	YAE_ASSERT(_resource->m_manager == nullptr);
	YAE_ASSERT(std::find(m_resources.begin(), m_resources.end(), _resource) == m_resources.end());

	YAE_VERIFY(string::safeCopyToBuffer(_resource->m_name, _name, countof(_resource->m_name)) < countof(_resource->m_name));

	// Register by name
	{
		StringHash nameHash = StringHash(_resource->m_name);	
		YAE_ASSERT(m_resourcesByName.get(nameHash) == nullptr);
		m_resourcesByName.set(nameHash, _resource);	
	}
	
	// Register by id
	{
		if (_resource->getID() == ResourceID::INVALID_ID)
		{
			u32 id = 0;
			size_t loopCount = 0;
			do
			{
				// unique ID is just a hash of the current time + reroll if it already exists.
				// hopefully it will be enough
				Time t = time::now();
				id = hash::hash32(&t, sizeof(t));
				++loopCount;
				YAE_ASSERT(loopCount < 32);
			}
			while (m_resourcesByID.get(id) != nullptr);
			_resource->m_id = id;
		}
		YAE_ASSERT(m_resourcesByID.get(_resource->getID()) == nullptr);
		m_resourcesByID.set(_resource->getID(), _resource);
	}

	// Register by type
	{
		const mirror::Class* clss = _resource->getClass();
		while (clss != nullptr)
		{
			DataArray<Resource*>* resourceArrayPtr = m_resourcesByType.getOrInsert(clss, DataArray<Resource*>());
			resourceArrayPtr->push_back(_resource);
			clss = clss->getParent();
		} 
	}

	_resource->m_manager = this;

	m_resources.push_back(_resource);
	YAE_VERBOSEF_CAT("resource", "Registered \"%s\"(%s)...", _resource->m_name, _resource->getClass()->getName());
}

void ResourceManager::unregisterResource(Resource* _resource)
{
	YAE_ASSERT(_resource != nullptr);
	YAE_ASSERT(_resource->m_manager == this);

	_resource->m_manager = nullptr;

	// unregister
	{
		auto it = std::find(m_resources.begin(), m_resources.end(), _resource);
		YAE_ASSERT(it != m_resources.end());
		m_resources.erase(it);
	}

	// unregister by type
	{
		const mirror::Class* clss = _resource->getClass();
		while (clss != nullptr)
		{
			DataArray<Resource*>* resourceArrayPtr = m_resourcesByType.get(clss);
			YAE_ASSERT(resourceArrayPtr != nullptr);
			Resource** resourcePtr = resourceArrayPtr->find(_resource);
			YAE_ASSERT(resourcePtr != nullptr);
			resourceArrayPtr->erase(resourcePtr);
			clss = clss->getParent();
		}
	}

	// unregister by name
	{
		StringHash nameHash = StringHash(_resource->m_name);	
		YAE_ASSERT(m_resourcesByName.get(nameHash) != nullptr);
		m_resourcesByName.remove(nameHash);
	}

	// unregister by id
	{
		YAE_ASSERT(m_resourcesByID.get(_resource->getID()) != nullptr);
		m_resourcesByID.remove(_resource->getID());
	}

	YAE_VERBOSEF_CAT("resource", "Unregistered \"%s\"...", _resource->m_name);
	_resource->m_name[0] = 0;
}

Resource* ResourceManager::findResource(const char* _name) const
{
	StringHash id = StringHash(_name);
	Resource*const* resourcePtr = m_resourcesByName.get(id);
	if (resourcePtr == nullptr)
		return nullptr;

	return *resourcePtr;
}

Resource* ResourceManager::findResource(ResourceID _id) const
{
	Resource*const* resourcePtr = m_resourcesByID.get(_id);
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

const DataArray<Resource*>& ResourceManager::getResources() const
{
	return m_resources;
}

const DataArray<Resource*>& ResourceManager::getResourcesByType(const mirror::Class* _class) const
{
	YAE_ASSERT(_class != nullptr);
	YAE_ASSERT(_class->isChildOf(mirror::GetClass<Resource>()));
	return *m_resourcesByType.get(_class);
}

void ResourceManager::flagResourceForReload(Resource* _resource)
{
	YAE_ASSERT(_resource->m_manager == this);

	m_resourcesToReloadMutex.lock();
	m_resourcesToReload.push_back(_resource);
	m_resourcesToReloadMutex.unlock();
}

void ResourceManager::registerReloadOnFileChanged(const char* _filePath, Resource* _resource)
{
	auto onFileChanged = [](const char* _filePath, FileChangeType _changeType, void* _userData)
	{
		if (_changeType == FileChangeType::MODIFIED)
		{
			Resource* resource = (Resource*)_userData;
			resource->requestReload();
			YAE_VERBOSEF_CAT("resource", "\"%s\" modified.", _filePath);
		}
	};

	engine().fileWatchSystem().startFileWatcher(_filePath, onFileChanged, _resource);
}

void ResourceManager::unregisterReloadOnFileChanged(const char* _filePath, Resource* _resource)
{
	engine().fileWatchSystem().stopFileWatcher(_filePath);
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
		resource->_reload();
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
	YAE_ASSERT(it != nullptr);
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
