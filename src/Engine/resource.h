#pragma once

#include <vector>
#include <unordered_map>

#include <types.h>
#include <hash.h>
#include <yae_string.h>

namespace yae {

typedef StringHash ResourceID;

extern const ResourceID UNDEFINED_RESOURCEID;



class YAELIB_API Resource
{
public:
	enum Error
	{
		ERROR_NONE,
		ERROR_LOAD
	};

	Resource(const char* _name);
	~Resource();
	
	ResourceID getID() const { return m_id; }
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
	T* findOrCreateResource(const char* _name)
	{
		ResourceID id(_name);
		auto it = m_resourcesByID.find(id);

		T* resource = nullptr;
		if (it == m_resourcesByID.end())
		{
			resource = new T(_name);
			registerResource(resource);
		}
		else
		{
			resource = reinterpret_cast<T*>(it->second);
		}
		return resource;
	}

private:
	std::vector<Resource*> m_resources;
	std::unordered_map<ResourceID, Resource*> m_resourcesByID;

	std::vector<Resource*> m_toDeleteResources; // work buffer;
};


template <typename T>
T* findOrCreateResource(const char* _name)
{
	return context().resourceManager->findOrCreateResource<T>(_name);
}


}