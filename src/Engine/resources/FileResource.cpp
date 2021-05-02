#include "FileResource.h"

#include <filesystem.h>
#include <context.h>

namespace yae {

MIRROR_CLASS_DEFINITION(FileResource);

FileResource::FileResource(const char* _path)
	: Resource(ResourceID(Path(_path, true, context().scratchAllocator).c_str()))
	, m_path(_path, true, context().defaultAllocator)
{
	setName(m_path.c_str());
}


FileResource::~FileResource()
{
	
}


Resource::Error FileResource::onLoaded(String& _outErrorDescription)
{
	FileHandle file(m_path.c_str());
	if (!file.open(FileHandle::OPENMODE_READ))
	{
		_outErrorDescription = "Could not open file.";
		return ERROR_LOAD;
	}

	m_contentSize = file.getSize();
	m_content = context().defaultAllocator->allocate(m_contentSize);
	file.read(m_content, m_contentSize);

	return ERROR_NONE;
}



void yae::FileResource::onUnloaded()
{
	context().defaultAllocator->deallocate(m_content);
	m_content = nullptr;
	m_contentSize = 0;
}

} // namespace yae
