#pragma once

#include <export.h>

#include <vector>
#include <unordered_map>

#include <00-Macro/Assert.h>
#include <00-Type/GlobalContext.h>

#include "ResourceID.h"

namespace yae {

class Resource;

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
T* FindOrCreateResource(const char* _name)
{
	GlobalContext* context = GetGlobalContext();
	YAE_ASSERT(context);
	YAE_ASSERT(context->resourceManager);
	return context->resourceManager->findOrCreateResource<T>(_name);
}

};
