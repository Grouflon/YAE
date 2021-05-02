#include "resource.h"

#include <yae/log.h>

namespace yae {

MIRROR_CLASS_DEFINITION(Resource);

const ResourceID UNDEFINED_RESOURCEID = ResourceID("");



Resource::Resource(ResourceID _id)
	: m_id(_id)
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
	: m_resources(context().defaultAllocator)
	, m_resourcesByID(context().defaultAllocator)
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
	YAE_ASSERT(m_resourcesByID.get(_resource->getID()) == nullptr);

	m_resources.push_back(_resource);
	m_resourcesByID.set(_resource->getID(), _resource);

	YAE_VERBOSEF_CAT("resource", "Registered \"%s\"...", _resource->getName());
}

void ResourceManager::unregisterResource(Resource* _resource)
{
	YAE_ASSERT(_resource != nullptr);

	{
		auto it = std::find(m_resources.begin(), m_resources.end(), _resource);
		YAE_ASSERT(it != m_resources.end());
		m_resources.erase(u32(it - m_resources.begin()), 1);
	}

	{
		YAE_ASSERT(m_resourcesByID.get(_resource->getID()) != nullptr);
		m_resourcesByID.remove(_resource->getID());
	}

	YAE_VERBOSEF_CAT("resource", "Unregistered \"%s\"...", _resource->getName());
}

void ResourceManager::flushResources()
{
	// Gather unused resources
	DataArray<Resource*> toDeleteResources(context().scratchAllocator);
	for (Resource* resource : m_resources)
	{
		if (!resource->isUsed())
		{
			toDeleteResources.push_back(resource);
		}
	}

	// Unregister & delete
	for (Resource* resource : toDeleteResources)
	{
		unregisterResource(resource);
		context().defaultAllocator->destroy(resource);
	}
	toDeleteResources.clear();
}



} // namespace yae
