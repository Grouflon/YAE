#include "resource.h"

#include <log.h>

namespace yae {

const ResourceID UNDEFINED_RESOURCEID = ResourceID("");



Resource::Resource(const char* _name)
	: m_id(ResourceID(_name))
	, m_name(_name)
{
	YAE_ASSERT(m_id != UNDEFINED_RESOURCEID);
}



Resource::~Resource()
{
	YAE_ASSERT(m_useCount == 0);
	YAE_ASSERT(m_loadCount == 0);
}



void Resource::use()
{
	++m_useCount;
}



bool Resource::load()
{
	if (m_loadCount == 0)
	{
		YAE_VERBOSEF_CAT("resource", "Loading \"%s\"...", getName());
		m_errorDescription = "";
		m_error = onLoaded(m_errorDescription);
		if (m_error != ERROR_NONE)
		{
			YAE_ERRORF_CAT("resource", "Failed to load \"%s\": %s", getName(), m_errorDescription.c_str());
		}
		else
		{
			YAE_LOGF_CAT("resource", "Loaded \"%s\"", getName());
		}
	}
	++m_loadCount;
	return m_error == ERROR_NONE;
}



bool Resource::useLoad()
{
	use();
	return load();
}



void Resource::unuse()
{
	YAE_ASSERT_MSG(m_useCount > 0, "Asymetric use/unuse detected");
	--m_useCount;
}



void Resource::release()
{
	YAE_ASSERT_MSG(m_useCount > 0, "Asymetric load/unload detected");
	--m_loadCount;
	if (m_loadCount == 0)
	{
		YAE_VERBOSEF_CAT("resource", "Releasing \"%s\"...", getName());
		onUnloaded();
		YAE_LOGF_CAT("resource", "Released \"%s\"", getName());
	}
}



void Resource::releaseUnuse()
{
	release();
	unuse();
}



ResourceManager::ResourceManager()
{
	if (context().resourceManager == nullptr)
	{
		context().resourceManager = this;
	}
}

ResourceManager::~ResourceManager()
{
	flushResources();

	YAE_ASSERT_MSG(m_resources.size() == 0, "Resources list must be empty when the manager gets destroyed");

	if (context().resourceManager == this)
	{
		context().resourceManager = nullptr;
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
	m_toDeleteResources.clear();
}



} // namespace yae
