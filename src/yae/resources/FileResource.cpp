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


void FileResource::_doLoad()
{
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(m_content == nullptr);
	YAE_ASSERT(m_contentSize == 0);

	FileHandle file(getName());
	if (!file.open(FileHandle::OPENMODE_READ))
	{
		_log(RESOURCELOGTYPE_ERROR, "Could not open file.");
		return;
	}

	m_contentSize = file.getSize();
	m_content = defaultAllocator().allocate(m_contentSize);
	file.read(m_content, m_contentSize);
	file.close();
}



void yae::FileResource::_doUnload()
{
	defaultAllocator().deallocate(m_content);
	m_content = nullptr;
	m_contentSize = 0;
}

} // namespace yae
