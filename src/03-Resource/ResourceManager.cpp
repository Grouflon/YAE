#include "ResourceManager.h"

#include <00-Macro/Assert.h>
#include <00-Type/GlobalContext.h>
#include <02-Log/Log.h>

#include "Resource.h"

namespace yae {

ResourceManager::ResourceManager()
{
	if (g_context.resourceManager == nullptr)
	{
		g_context.resourceManager = this;
	}
}

ResourceManager::~ResourceManager()
{
	flushResources();

	YAE_ASSERT_MSG(m_resources.size() == 0, "Resources list must be empty when the manager gets destroyed");

	if (g_context.resourceManager == this)
	{
		g_context.resourceManager = nullptr;
	}
}

void ResourceManager::registerResource(Resource* _resource)
{
	YAE_ASSERT(_resource != nullptr);
	YAE_ASSERT(std::find(m_resources.begin(), m_resources.end(), _resource) == m_resources.end());
	YAE_ASSERT(m_resourcesByID.find(_resource->getID()) == m_resourcesByID.end());

	m_resources.push_back(_resource);
	m_resourcesByID.insert(std::make_pair(_resource->getID(), _resource));

	YAE_VERBOSEF_CAT("resource", "Registered \"%s\"...", _resource->getName());
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
		auto it = m_resourcesByID.find(_resource->getID());
		YAE_ASSERT(it != m_resourcesByID.end());
		m_resourcesByID.erase(it);
	}

	YAE_VERBOSEF_CAT("resource", "Unregistered \"%s\"...", _resource->getName());
}

void ResourceManager::flushResources()
{
	// Gather unused resources
	for (Resource* resource : m_resources)
	{
		if (!resource->isUsed())
		{
			m_toDeleteResources.push_back(resource);
		}
	}

	// Unregister & delete
	for (Resource* resource : m_toDeleteResources)
	{
		unregisterResource(resource);
		delete resource;
	}
	m_toDeleteResources.empty();
}

}
