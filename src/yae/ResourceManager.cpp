#include "ResourceManager.h"

#include <yae/string.h>
#include <yae/resource.h>
#include <yae/resources/Resource.h>

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

void ResourceManager::reloadResource(Resource* _resource)
{
	if (!_resource->isLoaded())
		return;

	_resource->_internalUnload();
	_resource->_internalLoad();
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

} // namespace yae
