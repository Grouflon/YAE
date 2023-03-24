#pragma once

#include <yae/types.h>

#include <yae/containers/Array.h>

#include <mirror/mirror.h>

namespace yae {

enum ResourceLogType
{
	RESOURCELOGTYPE_LOG,
	RESOURCELOGTYPE_WARNING,
	RESOURCELOGTYPE_ERROR,
};

struct YAE_API ResourceLog
{
	ResourceLogType type;
	String message;
};

class YAE_API Resource
{
	MIRROR_CLASS(Resource)
	(
	);

public:
	Resource();
	virtual ~Resource();
	
	const char* getName() const { return m_name; }

	bool load();
	void release();

	bool isLoaded() const { return m_loadCount > 0 && m_errorCount == 0; } // @TODO warning as errors option ?

	const Array<ResourceLog>& getLogs() const { return m_logs; }

// private:
	void _internalLoad();
	void _internalUnload();

	virtual void _doLoad() {}
	virtual void _doUnload() {}

	void _log(ResourceLogType _type, const char* _msg);

	Array<ResourceLog> m_logs;
	char m_name[256];
	u32 m_loadCount = 0;
	u32 m_errorCount = 0;
	u32 m_warningCount = 0;
	bool m_isLoading = false;
};

} // namespace yae