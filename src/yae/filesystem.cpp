#include "filesystem.h"

#include <yae/program.h>
#include <yae/platform.h>
#include <yae/containers/Array.h>

#include <filesystem>
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


String getAbsolutePath(const char* _path)
{
	return normalizePath(platform::getAbsolutePath(_path).c_str());
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
	return normalizePath(platform::getWorkingDirectory().c_str());
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


String getFileName(const char* _path)
{
	String path(_path, &scratchAllocator());
	normalizePath(_path);
	size_t size = path.size();
	for (size_t i = 0; i < size; ++i)
	{
		size_t pos = size - 1 - i;
		if (path[pos] == '/')
		{
			return path.slice(pos + 1, size);
		}
	}
	return path;
}


String getFileNameWithoutExtension(const char* _path)
{
	String fileName(getFileName(_path), &scratchAllocator());
	size_t size = fileName.size();
	for (size_t i = 0; i < size; ++i)
	{
		size_t pos = size - 1 - i;
		if (fileName[pos] == '.')
		{
			return fileName.slice(0, pos);
		}
	}
	return fileName;
}


String getExtension(const char* _path)
{
	String fileName(getFileName(_path), &scratchAllocator());
	size_t size = fileName.size();
	for (size_t i = 0; i < size; ++i)
	{
		size_t pos = size - 1 - i;
		if (fileName[pos] == '.')
		{
			return fileName.slice(pos + 1, size);
		}
	}
	return String();
}


bool doesPathExists(const char* _path)
{
	std::error_code errorCode;
	bool result = std::filesystem::exists(_path, errorCode);
	YAE_ASSERT(errorCode.value() == 0);
	return result;
}


bool deletePath(const char* _path)
{
	std::error_code errorCode;
	std::uintmax_t ret = std::filesystem::remove_all(_path, errorCode);
	return ret > 0 && ret != static_cast<std::uintmax_t>(-1);
}


bool createDirectory(const char* _path)
{
	return std::filesystem::create_directory(_path);
}


Date getFileLastWriteTime(const char* _path)
{
#if YAE_PLATFORM_WEB == 0
	std::error_code errorCode;
	auto ftime = std::filesystem::last_write_time(_path, errorCode);
	if (errorCode.value() != 0)
		return Date(0);

	using namespace std::chrono;

	auto sctp = time_point_cast<system_clock::duration>(ftime - decltype(ftime)::clock::now() + system_clock::now());
    std::time_t cftime = system_clock::to_time_t(sctp);

    //std::time_t cftime = decltype(ftime)::clock::to_time_t(ftime);
    return Date(i64(cftime));
#else
    YAE_ASSERT_MSG(false, "getFileLastWriteTime is not supported for Web yet");
    return Date(0);
#endif
}


bool copy(const char* _from, const char* _to, CopyMode _mode)
{
	using namespace std::filesystem;
	copy_options copyOptions = copy_options::none;
	switch(_mode)
	{
		case CopyMode_SkipExisting: copyOptions = copy_options::skip_existing; break;
		case CopyMode_OverwriteExisting: copyOptions = copy_options::overwrite_existing; break;
		case CopyMode_OverwriteExistingIfOlder: copyOptions = copy_options::update_existing; break;
	}

	std::error_code errorCode;
	std::filesystem::copy(_from, _to, copyOptions, errorCode);
	return errorCode.value() == 0;
}

bool internalWalkDirectory(const char* _path, bool(*_visitor)(const Entry& _entry, void* _userData), bool _recursive, EntryType _filter, void* _userData)
{
	bool continueWalk = true;

	for (const auto& directoryEntry : std::filesystem::directory_iterator(_path))
	{
		Entry entry;
		entry.path = normalizePath(directoryEntry.path().string().c_str());

		if (directoryEntry.is_directory())
		{
			if (_filter & EntryType_Directory)
			{
				entry.type = EntryType_Directory;
				continueWalk = _visitor(entry, _userData);
			}

			if (_recursive && continueWalk)
			{
				continueWalk = internalWalkDirectory(entry.path.c_str(), _visitor, true, _filter, _userData);
			}
		}
		else if (directoryEntry.is_symlink())
		{
			if (_filter & EntryType_Symlink)
			{
				entry.type = EntryType_Symlink;
				continueWalk = _visitor(entry, _userData);
			}
		}
		else if (directoryEntry.is_regular_file())
		{
			if (_filter & EntryType_File)
			{
				entry.type = EntryType_File;
				continueWalk = _visitor(entry, _userData);
			}
		}
		if (!continueWalk)
			break;
	}
	return continueWalk;
};

void walkDirectory(const char* _path, bool(*_visitor)(const Entry& _entry, void* _userData), bool _recursive, EntryType _filter, void* _userData)
{
	YAE_ASSERT(_visitor != nullptr);

	if (!std::filesystem::exists(_path))
		return;

	String256 path = filesystem::normalizePath(_path);
	internalWalkDirectory(path.c_str(), _visitor, _recursive, _filter, _userData);
}

void parseDirectoryContent(const char* _path, Array<Entry>& _outContent, bool _recursive, EntryType _filter)
{
	walkDirectory(_path, [](const Entry& _entry, void* _userData)
	{
		Array<Entry>* outContentPtr = (Array<Entry>*)_userData;
		outContentPtr->push_back(_entry);
		return true;
	}
	, _recursive
	, _filter
	, &_outContent);
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

FileReader::FileReader(const char* _path, Allocator* _allocator)
{
	m_path = filesystem::normalizePath(_path);
	m_allocator = _allocator;
}

FileReader::~FileReader()
{
	allocator().deallocate(m_content);
	m_content = nullptr;
}

bool FileReader::load()
{
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(!m_isLoaded);
	YAE_ASSERT(m_contentSize == 0);
	YAE_ASSERT(m_content == nullptr);

	FileHandle file(m_path.c_str());
	if (!file.open(FileHandle::OPENMODE_READ))
	{
		YAE_ERRORF("failed to open file \"%s\".", m_path.c_str());
		return false;
	}

	m_contentSize = file.getSize();
	m_content = allocator().allocate(m_contentSize);
	file.read(m_content, m_contentSize);
	file.close();
	
	m_isLoaded = true;
	return true;
}

u32 FileReader::getContentSize() const
{
	YAE_ASSERT(m_isLoaded);
	return m_contentSize;
}

const void* FileReader::getContent() const
{
	YAE_ASSERT(m_isLoaded);
	return m_content;
}

const char* FileReader::getPath() const
{
	return m_path.c_str();
}

Allocator& FileReader::allocator() const
{
	return m_allocator ? *m_allocator : defaultAllocator();
}

} // namespace yae
