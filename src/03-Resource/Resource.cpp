#include "Resource.h"

#include <00-Macro/Assert.h>
#include <02-Log/Log.h>

yae::Resource::Resource(const char* _name)
	: m_id(ResourceID(_name))
	, m_name(_name)
{
	YAE_ASSERT(m_id != UNDEFINED_RESOURCEID);
}

yae::Resource::~Resource()
{
	YAE_ASSERT(m_useCount == 0);
	YAE_ASSERT(m_loadCount == 0);
}

void yae::Resource::use()
{
	++m_useCount;
}

void yae::Resource::load()
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
}

void yae::Resource::useLoad()
{
	use();
	load();
}

void yae::Resource::unuse()
{
	YAE_ASSERT_MSG(m_useCount > 0, "Asymetric use/unuse detected");
	--m_useCount;
}

void yae::Resource::release()
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

void yae::Resource::releaseUnuse()
{
	release();
	unuse();
}
