#pragma once

#include <yae/types.h>
#include <yae/containers/HashMap.h>


namespace yae {

class Resource;

class YAE_API ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	void registerResource(const char* _name, Resource* _resource);
	void unregisterResource(Resource* _resource);

	void reloadResource(Resource* _resource);

	void flushResources();

	Resource* findResource(const char* _name) const;
	template <typename T> T* findResource(const char* _name) const;

	const DataArray<Resource*> getResources() const;

private:
	DataArray<Resource*> m_resources;
	HashMap<StringHash, Resource*> m_resourcesByID;
};

} // namespace yae

#include "ResourceManager.inl"
