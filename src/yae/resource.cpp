#include "resource.h"

#include <yae/program.h>

namespace yae {

MIRROR_CLASS_DEFINITION(Resource);

const ResourceID UNDEFINED_RESOURCEID = ResourceID("");


Resource::Resource(ResourceID _id)
	: m_id(_id)
{
	YAE_ASSERT(m_id != UNDEFINED_RESOURCEID);
}

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

		// Reset Logs
		m_errorCount = 0;
		m_warningCount = 0;
		m_logs.clear();

		// Load
		m_isLoading = true;
		_doLoad();
		m_isLoading = false;

		for (const ResourceLog& log : m_logs)
		{
			switch(log.type)
			{
				case RESOURCELOGTYPE_LOG:
					YAE_VERBOSEF_CAT("resource", "[%s] %s", getName(), log.message.c_str());
					break;
				case RESOURCELOGTYPE_WARNING:
					YAE_WARNINGF_CAT("resource", "[%s] %s", getName(), log.message.c_str());
					break;
				case RESOURCELOGTYPE_ERROR:
					YAE_ERRORF_CAT("resource", "[%s] %s", getName(), log.message.c_str());
					break;
			}
		}

		if (m_errorCount > 0)
		{
			YAE_ERRORF_CAT("resource", "Error Loading \"%s\" (%d warnings, %d errors)", getName(), m_warningCount, m_errorCount);
		}
		else if (m_warningCount > 0)
		{
			YAE_LOGF_CAT("resource", "Loaded \"%s\" with warnings (%d warnings, %d errors)", getName(), m_warningCount, m_errorCount);
		}
		else
		{
			YAE_LOGF_CAT("resource", "Loaded \"%s\" (%d warnings, %d errors)", getName(), m_warningCount, m_errorCount);
		}
	}
	++m_loadCount;
	return m_errorCount == 0;
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
		_doUnload();
		YAE_LOGF_CAT("resource", "Released \"%s\"", getName());
	}
}


void Resource::releaseUnuse()
{
	release();
	unuse();
}


void Resource::_log(ResourceLogType _type, const char* _msg)
{
	YAE_ASSERT_MSG(m_isLoading, "You can only log during the _doLoad call");
	
	ResourceLog log;
	log.type = _type;
	log.message = _msg;
	m_logs.push_back(log);

	switch(_type)
	{
		case RESOURCELOGTYPE_WARNING: ++m_warningCount; break;
		case RESOURCELOGTYPE_ERROR: ++m_errorCount; break;
	}
}


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
	DataArray<Resource*> toDeleteResources(&scratchAllocator());
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
		defaultAllocator().destroy(resource);
	}
	toDeleteResources.clear();
}


} // namespace yae
