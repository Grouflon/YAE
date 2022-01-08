#pragma once

#include <yae/types.h>
#include <yae/hash.h>
#include <yae/containers/HashMap.h>

#include <mirror/mirror.h>

namespace yae {

typedef StringHash ResourceID;

YAELIB_API extern const ResourceID UNDEFINED_RESOURCEID;


enum ResourceLogType
{
	RESOURCELOGTYPE_LOG,
	RESOURCELOGTYPE_WARNING,
	RESOURCELOGTYPE_ERROR,
};

struct YAELIB_API ResourceLog
{
	ResourceLogType type;
	String message;
};

class YAELIB_API Resource
{
	MIRROR_CLASS(Resource)
	(
	);

public:

	Resource(ResourceID _id);
	Resource(const char* _name);
	virtual ~Resource();
	
	ResourceID getID() const { return m_id; }
	const char* getName() const { return m_name.c_str(); }

	void use();
	bool load();
	bool useLoad();
	void unuse();
	void release();
	void releaseUnuse();

	bool isLoaded() const { return m_loadCount > 0 && m_errorCount == 0; } // @TODO warning as errors option ?
	bool isUsed() const { return m_useCount > 0; }

	const Array<ResourceLog>& getLogs() const { return m_logs; }

// private:
	void _internalLoad();
	void _internalUnload();

	virtual void _doLoad() {}
	virtual void _doUnload() {}

	void _log(ResourceLogType _type, const char* _msg);

	Array<ResourceLog> m_logs;
	String m_name;
	ResourceID m_id;
	u32 m_useCount = 0;
	u32 m_loadCount = 0;
	u32 m_errorCount = 0;
	u32 m_warningCount = 0;
	bool m_isLoading = false;
};



class YAELIB_API ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	void registerResource(Resource* _resource);
	void unregisterResource(Resource* _resource);

	void reloadResource(Resource* _resource);

	void flushResources();

	template <typename T>
	T* findResource(ResourceID _id)
	{
		Resource** resourcePtr = m_resourcesByID.get(_id);
		if (resourcePtr == nullptr)
			return nullptr;

		return mirror::Cast<T*>(*resourcePtr);
	}

private:
	DataArray<Resource*> m_resources;
	HashMap<ResourceID, Resource*> m_resourcesByID;
};

template <typename T, typename ...Args>
T* findOrCreateResource(Args... _args)
{
	ResourceManager& manager = resourceManager();
	ResourceID id = ResourceIDGetter<T>::GetId(_args...);
	YAE_ASSERT_MSG(id != UNDEFINED_RESOURCEID, "Unknown Resource Type");
	T* resource = manager.findResource<T>(id);
	if (resource == nullptr)
	{
		resource = defaultAllocator().create<T>(_args...);
		manager.registerResource(resource);
	}
	return resource;
}

template <typename T, typename ...Args>
T* reloadResource(Args... _args)
{
	ResourceManager& manager = resourceManager();
	ResourceID id = ResourceIDGetter<T>::GetId(_args...);
	YAE_ASSERT_MSG(id != UNDEFINED_RESOURCEID, "Unknown Resource Type");
	T* resource = manager.findResource<T>(id);
	if (resource != nullptr)
	{
		manager.reloadResource(resource);
	}
	return resource;
}

template <typename T, typename ...Args>
struct ResourceIDGetter
{
	static ResourceID GetId(Args... _args) { return UNDEFINED_RESOURCEID; }
};


}