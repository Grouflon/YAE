#pragma once

#include <yae/types.h>

namespace yae {

typedef Resource* (*AllocateResourceFunction)(Allocator* _allocator);
typedef void (*DeallocateResourceFunction)(Allocator* _allocator, Resource* _resource);

struct ResourceTypeDescriptor
{
	String name;
	AllocateResourceFunction allocateFunction = nullptr;
	DeallocateResourceFunction deallocateFunction = nullptr;
};

class YAE_API ResourceManager2
{
public:
	ResourceManager2() {}

	void registerResourceType(const ResourceTypeDescriptor& _descriptor)
	{
		StringHash hash = StringHash(_descriptor.name.c_str());
		YAE_ASSERT(m_resourceTypes.get(hash) == nullptr);
		m_resourceTypes.set(hash, _descriptor);
	}

	void unregisterResourceType(const char* _name)
	{
		StringHash hash = StringHash(_name);
		YAE_ASSERT(m_resourceTypes.get(hash) != nullptr);
		m_resourceTypes.remove(hash);
	}

//private:
	HashMap<StringHash, ResourceTypeDescriptor> m_resourceTypes;
};

#define REGISTER_RESOURCE_TYPE(ResourceType) \
do { \
	yae::ResourceTypeDescriptor __descriptor; \
	__descriptor.name = #ResourceType; \
	__descriptor.allocateFunction = [](yae::Allocator* _allocator) -> yae::Resource* \
	{ \
		return _allocator->create<ResourceType>(); \
	}; \
	__descriptor.deallocateFunction = [](yae::Allocator* _allocator, Resource* _resource) \
	{ \
		return _allocator->destroy(_resource); \
	}; \
	yae::program().resourceManager2().registerResourceType(__descriptor); \
} while(0)\

} // namespace yae
