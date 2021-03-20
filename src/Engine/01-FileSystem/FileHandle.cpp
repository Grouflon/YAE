#include "FileHandle.h"

namespace yae {


FileHandle::FileHandle(const char* path)
	: m_file(nullptr)
{
	m_path = path;
}

FileHandle::~FileHandle()
{
	close();
}

bool FileHandle::open(OpenMode mode)
{
	if (m_file && mode != m_openMode)
	{
		close();
	}

	m_openMode = mode;
	const char* modeStr = "";
	switch (m_openMode)
	{
	case OPENMODE_READ:
		{
			modeStr = "rb";
		} break;

	case OPENMODE_WRITE:
		{
			modeStr = "wb";
		} break;

	case OPENMODE_APPEND:
		{
			modeStr = "ab";
		} break;

	default:
		break;
	}

	m_file = std::fopen(m_path.c_str(), modeStr);

	if (!m_file)
	{
		return false;
	}

	return true;
}

void FileHandle::close()
{
	if (m_file)
	{
		std::fclose(m_file);
		m_file = nullptr;
	}
}

bool FileHandle::write(const void* buffer, size_t size)
{
	if (m_file)
	{
		return size == 0 || std::fwrite(buffer, size, 1, m_file) != 0;
	}
	return false;
}

size_t FileHandle::read(void* buffer, size_t size) const
{
	if (m_file)
	{
		return std::fread(buffer, 1, size, m_file);
	}
	return 0;
}

bool FileHandle::isOpen() const
{
	return m_file != nullptr;
}

i32 FileHandle::getSize() const
{
	if (m_file)
	{
		i32 offset = std::ftell(m_file);
		std::fseek(m_file, 0, SEEK_END);
		i32 size = ftell(m_file);
		std::fseek(m_file, offset, SEEK_SET);
		return size;
	}
	return 0;
}

i32 FileHandle::getOffset() const
{
	if (m_file)
	{
		return std::ftell(m_file);
	}
	return 0;
}

void FileHandle::setOffset(i32 offset)
{
	if (m_file)
	{
		std::fseek(m_file, offset, SEEK_SET);
	}
}

const char* FileHandle::getPath() const
{
	return m_path.c_str();
}

} // namespace yae
