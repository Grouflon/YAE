#include "FileResource.h"

#include <01-FileSystem/Path.h>
#include <01-FileSystem/FileHandle.h>

namespace yae {

yae::FileResource::FileResource(const char* _path)
	: Resource(Path(_path, true).getCString())
{
}

Resource::Error FileResource::onLoaded(std::string& _outErrorDescription)
{
	FileHandle file(getName());
	if (!file.open(FileHandle::OPENMODE_READ))
	{
		_outErrorDescription = "Could not open file.";
		return ERROR_LOAD;
	}

	m_contentSize = file.getSize();
	m_content = new u8[m_contentSize];
	file.read(m_content, m_contentSize);

	return ERROR_NONE;
}

void yae::FileResource::onUnloaded()
{
	delete[] m_content;
	m_content = nullptr;
	m_contentSize = 0;
}

}
