#pragma once

#include <yae/types.h>
#include <yae/hash.h>
#include <yae/string.h>
#include <yae/containers.h>

#include <mirror/mirror.h>

namespace yae {

typedef StringHash ResourceID;

extern const ResourceID UNDEFINED_RESOURCEID;


class YAELIB_API Resource
{
	MIRROR_CLASS(Resource)
	(
	);

public:
	enum Error
	{
		ERROR_NONE,
		ERROR_LOAD
	};

	Resource(ResourceID _id);
	virtual ~Resource();
	
	ResourceID getID() const { return m_id; }
	void setName(const char* _name) { m_name = _name; }
	const char* getName() const { return m_name.c_str(); }

	void use();
	bool load();
	bool useLoad();
	void unuse();
	void release();
	void releaseUnuse();

	bool isLoaded() const { return m_loadCount > 0; }
	bool isUsed() const { return m_useCount > 0; }

	Error getError() const { return m_error; }
	const char* getErrorDescription() const { return m_errorDescription.c_str(); }

protected:
	virtual Error onLoaded(String& _outErrorDescription) { return ERROR_NONE; }
	virtual void onUnloaded() {}

private:
	ResourceID m_id;
	String m_name;
	u32 m_useCount = 0;
	u32 m_loadCount = 0;

	Error m_error = ERROR_NONE;
	String m_errorDescription;
};



class YAELIB_API ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	void registerResource(Resource* _resource);
	void unregisterResource(Resource* _resource);

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
	T* resource = manager.findResource<T>(id);
	if (resource == nullptr)
	{
		resource = defaultAllocator().create<T>(_args...);
		manager.registerResource(resource);
	}
	return resource;
}

template <typename T, typename ...Args>
struct ResourceIDGetter
{
	static ResourceID GetId(Args... _args) { return UNDEFINED_RESOURCEID; }
};


}