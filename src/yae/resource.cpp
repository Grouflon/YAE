#include "resource.h"

#include <yae/filesystem.h>
#include <yae/resources/Resource.h>
#include <yae/serialization/JsonSerializer.h>
#include <yae/serialization/serialization.h>

#include <mirror.h>

namespace yae {
namespace resource {

Resource* findOrCreateFromFile(const char* _path)
{
	ResourceManager& manager = resourceManager();

	Resource* resource = manager.findResource(_path);
	if (resource == nullptr)
	{
		FileReader reader(_path, &scratchAllocator());
		if (!reader.load())
		{
			YAE_ERRORF_CAT("resource", "Failed to open \"%s\" for read", _path);
			return nullptr;
		}

		JsonSerializer serializer(&scratchAllocator());
		if (!serializer.parseSourceData(reader.getContent(), reader.getContentSize()))
		{
			YAE_ERRORF_CAT("resource", "Failed to parse \"%s\" JSON file", _path);
			return nullptr;
		}

		serializer.beginRead();
		YAE_VERIFY(serializer.beginSerializeObject());
		String resourceTypeStr(&scratchAllocator());
		YAE_VERIFY(serializer.serialize(resourceTypeStr, "type"));
		mirror::Class* resourceType = mirror::FindClassByName(resourceTypeStr.c_str());
		if (resourceType != nullptr)
		{
			YAE_ASSERT(resourceType->hasFactory());
			resource = (Resource*) resourceType->instantiate([](size_t _size, void*) { return defaultAllocator().allocate(_size); });
			YAE_ASSERT(resource != nullptr);

			YAE_VERIFY(serialization::serializeClassInstanceMembers(&serializer, resource, resourceType));
		}
		else
		{
			YAE_ERRORF_CAT("resource", "Unknown reflected type \"%s\"", resourceTypeStr.c_str());
		}
		YAE_VERIFY(serializer.endSerializeObject());
		serializer.endRead();

		resource->m_transient = false;
		manager.registerResource(_path, resource);
	}
	return resource;
}

void saveToFile(Resource* _resource, const char* _path)
{
	YAE_ASSERT(_resource != nullptr);

	mirror::Class* resourceType = _resource->getClass();

	JsonSerializer serializer(&scratchAllocator());
	serializer.beginWrite();
	YAE_VERIFY(serializer.beginSerializeObject());
	String resourceTypeStr = String(resourceType->getName(), &scratchAllocator());
	YAE_VERIFY(serializer.serialize(resourceTypeStr, "type"));
	YAE_VERIFY(serialization::serializeClassInstanceMembers(&serializer, const_cast<Resource*>(_resource), resourceType));
	YAE_VERIFY(serializer.endSerializeObject());
	serializer.endWrite();

	FileHandle file(_path);
	if (!file.open(FileHandle::OPENMODE_WRITE))
	{
		YAE_ERRORF_CAT("resource", "Failed to open \"%s\" for write", _path);
		return;
	}
	if (!file.write(serializer.getWriteData(), serializer.getWriteDataSize()))
	{
		YAE_ERRORF_CAT("resource", "Failed to write into \"%s\"", _path);
		return;
	}
	file.close();
}

} // namespace resource
} // namespace yae
