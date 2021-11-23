#include "FileResource.h"

#include <yae/filesystem.h>
#include <yae/program.h>

namespace yae {

MIRROR_CLASS_DEFINITION(FileResource);

FileResource::FileResource(const char* _path)
	: Resource(filesystem::normalizePath(_path).c_str())
{
}


FileResource::~FileResource()
{
}


Resource::Error FileResource::_doLoad(String& _outErrorDescription)
{
	FileHandle file(getName());
	if (!file.open(FileHandle::OPENMODE_READ))
	{
		_outErrorDescription = "Could not open file.";
		return ERROR_LOAD;
	}

	m_contentSize = file.getSize();
	m_content = defaultAllocator().allocate(m_contentSize);
	file.read(m_content, m_contentSize);

	return ERROR_NONE;
}



void yae::FileResource::_doUnload()
{
	defaultAllocator().deallocate(m_content);
	m_content = nullptr;
	m_contentSize = 0;
}

} // namespace yae
