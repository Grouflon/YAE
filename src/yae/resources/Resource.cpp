#include "Resource.h"

namespace yae {

MIRROR_CLASS_DEFINITION(Resource);

Resource::Resource()
{
	
}

Resource::~Resource()
{
	YAE_ASSERT(m_loadCount == 0);
}

bool Resource::load()
{
	if (m_loadCount == 0)
	{
		_internalLoad();
	}
	++m_loadCount;
	return m_errorCount == 0;
}

void Resource::release()
{
	YAE_ASSERT_MSG(m_loadCount > 0, "Asymetric load/unload detected");
	--m_loadCount;
	if (m_loadCount == 0)
	{
		_internalUnload();
	}
}

void Resource::_internalLoad()
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


void Resource::_internalUnload()
{
	YAE_VERBOSEF_CAT("resource", "Releasing \"%s\"...", getName());
	_doUnload();
	YAE_LOGF_CAT("resource", "Released \"%s\"", getName());
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
		default: break;
	}
}

} // namespace yae
