#include "File.h"

#include <core/filesystem.h>
#include <core/string.h>

MIRROR_CLASS(yae::File)
(
	MIRROR_PARENT(yae::Resource)
);

namespace yae {

File::File()
{
	
}

File::~File()
{
}

void File::setPath(const char* _path)
{
	YAE_ASSERT(!isLoaded());
	m_path = filesystem::normalizePath(_path);
}
const char* File::getPath() const
{
	return m_path.c_str();
}

const void* File::getContent() const
{
	return m_content;
}

size_t File::getContentSize() const
{
	return m_contentSize;
}

void File::_doLoad()
{
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(m_content == nullptr);
	YAE_ASSERT(m_contentSize == 0);

	FileHandle file(m_path.c_str());
	if (!file.open(FileHandle::OPENMODE_READ))
	{
		_log(RESOURCELOGTYPE_ERROR, string::format("Could not open file \"%s\".", m_path.c_str()).c_str());
		return;
	}

	m_contentSize = file.getSize();
	m_content = defaultAllocator().allocate(m_contentSize);
	file.read(m_content, m_contentSize);
	file.close();
}



void yae::File::_doUnload()
{
	defaultAllocator().deallocate(m_content);
	m_content = nullptr;
	m_contentSize = 0;
}

} // namespace yae
