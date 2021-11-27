#include "filesystem.h"

#include <yae/program.h>
#include <yae/platform.h>

#include <experimental/filesystem>
#include <cstdio>

namespace yae {

namespace filesystem {

String normalizePath(const char* _path)
{
	String path(_path, &scratchAllocator());
	return normalizePath(path);
}


String& normalizePath(String& _path)
{
	for (u32 i = 0; i < _path.size();)
	{
		// Only forward slashes
		if (_path[i] == '\\')
		{
			_path[i] = '/';
		}

		// Remove all double slashes
		if (i > 0 && _path[i - 1] == '/' && _path[i] == '/')
		{
			_path.replace(i, 1, "");
			continue;
		}
		++i;
	}
	return _path;
}


String getDirectory(const char* _path)
{
	String path(_path, &scratchAllocator());
	normalizePath(_path);
	size_t size = path.size();
	for (size_t i = 0; i < size; ++i)
	{
		size_t pos = size - 1 - i;
		if (path[pos] == '/')
		{
			return path.slice(0, pos + 1);
		}
	}
	return path;
}


String getAbsolutePath(const char* _path)
{
	return normalizePath(platform::getAbsolutePath(_path));
}


String getRelativePath(const char* _path, const char* _relativeTo)
{
	return String(_path, &scratchAllocator());
}


void setWorkingDirectory(const char* _path)
{
	platform::setWorkingDirectory(_path);
}


String getWorkingDirectory()
{
	return normalizePath(platform::getWorkingDirectory());
}


bool deletePath(const char* _path)
{
	std::error_code errorCode;
	std::uintmax_t ret = std::experimental::filesystem::remove_all(_path, errorCode);
	return ret > 0 && ret != static_cast<std::uintmax_t>(-1);
}


bool createDirectory(const char* _path)
{
	return std::experimental::filesystem::create_directory(_path);
}


bool copy(const char* _from, const char* _to, CopyMode _mode)
{
	using namespace std::experimental::filesystem;
	copy_options copyOptions = copy_options::none;
	switch(_mode)
	{
		case CopyMode_SkipExisting: copyOptions = copy_options::skip_existing; break;
		case CopyMode_OverwriteExisting: copyOptions = copy_options::overwrite_existing; break;
		case CopyMode_OverwriteExistingIfOlder: copyOptions = copy_options::update_existing; break;
	}

	std::error_code errorCode;
	std::experimental::filesystem::copy(_from, _to, copyOptions, errorCode);
	return errorCode.value() == 0;
}

} // namespace filesystem


FileHandle::FileHandle(const char* path)
	: m_path(path)
	, m_fileHandle(nullptr)
{
}

FileHandle::~FileHandle()
{
	close();
}

bool FileHandle::open(OpenMode mode)
{
	if (m_fileHandle && mode != m_openMode)
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

	m_fileHandle = std::fopen(m_path.c_str(), modeStr);

	if (m_fileHandle == nullptr)
	{
		return false;
	}

	return true;
}

void FileHandle::close()
{
	if (m_fileHandle)
	{
		std::fclose((std::FILE*)m_fileHandle);
		m_fileHandle = nullptr;
	}
}

bool FileHandle::write(const void* buffer, size_t size)
{
	if (m_fileHandle)
	{
		return size == 0 || std::fwrite(buffer, size, 1, (std::FILE*)m_fileHandle) != 0;
	}
	return false;
}

size_t FileHandle::read(void* buffer, size_t size) const
{
	if (m_fileHandle)
	{
		return std::fread(buffer, 1, size, (std::FILE*)m_fileHandle);
	}
	return 0;
}

bool FileHandle::isOpen() const
{
	return m_fileHandle != nullptr;
}

i32 FileHandle::getSize() const
{
	if (m_fileHandle)
	{
		i32 offset = std::ftell((std::FILE*)m_fileHandle);
		std::fseek((std::FILE*)m_fileHandle, 0, SEEK_END);
		i32 size = ftell((std::FILE*)m_fileHandle);
		std::fseek((std::FILE*)m_fileHandle, offset, SEEK_SET);
		return size;
	}
	return 0;
}

i32 FileHandle::getOffset() const
{
	if (m_fileHandle)
	{
		return std::ftell((std::FILE*)m_fileHandle);
	}
	return 0;
}

void FileHandle::setOffset(i32 offset)
{
	if (m_fileHandle)
	{
		std::fseek((std::FILE*)m_fileHandle, offset, SEEK_SET);
	}
}

const char* FileHandle::getPath() const
{
	return m_path.c_str();
}

} // namespace yae
