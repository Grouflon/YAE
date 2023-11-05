#pragma once

#include <core/types.h>

#include <core/Date.h>

// interesting stuff: http://utf8everywhere.org/

namespace yae {

class Allocator;
template <typename T> class Array;

namespace filesystem {

// Path management
CORE_API String normalizePath(const char* _path);
CORE_API String& normalizePath(String& _path);
CORE_API String getAbsolutePath(const char* _path);
CORE_API String getRelativePath(const char* _path, const char* _relativeTo);
CORE_API void setWorkingDirectory(const char* _path);
CORE_API String getWorkingDirectory();
CORE_API String getDirectory(const char* _path);
CORE_API String getFileName(const char* _path);
CORE_API String getFileNameWithoutExtension(const char* _path);
CORE_API String getExtension(const char* _path);

// Filesystem actions
CORE_API bool doesPathExists(const char* _path);
CORE_API bool deletePath(const char* _path);
CORE_API bool createDirectory(const char* _path);
CORE_API Date getFileLastWriteTime(const char* _path);

enum CopyMode
{
	CopyMode_SkipExisting,
	CopyMode_OverwriteExisting,
	CopyMode_OverwriteExistingIfOlder,
};
CORE_API bool copy(const char* _from, const char* _to, CopyMode _mode = CopyMode_SkipExisting);

typedef i32 EntryType;
enum EntryType_
{
	EntryType_File = 1 << 0,
	EntryType_Directory = 1 << 1,
	EntryType_Symlink = 2 << 2,

	EntryType_All = EntryType_File|EntryType_Directory|EntryType_Symlink,
};
struct Entry
{
	String256 path;
	EntryType type;
};

CORE_API void walkDirectory(const char* _path, bool(*_visitor)(const Entry& _entry, void* _userData), bool _recursive = true, EntryType _filter = EntryType_All, void* _userData = nullptr);
CORE_API void parseDirectoryContent(const char* _path, Array<Entry>& _outContent, bool _recursive = true, EntryType _filter = EntryType_All);

} // namespace filesystem

class CORE_API FileHandle
{
public:
	enum OpenMode
	{
		OPENMODE_READ,
		OPENMODE_WRITE,
		OPENMODE_APPEND
	};

	FileHandle(const char* path);
	~FileHandle();

	bool open(OpenMode mode);
	void close();

	bool write(const void* buffer, size_t size);
	size_t read(void* buffer, size_t size) const;

	bool isOpen() const;
	i32 getSize() const;
	i32 getOffset() const;
	void setOffset(i32 offset);
	const char* getPath() const;

private:
	String      m_path;
	OpenMode    m_openMode;
	void*       m_fileHandle;
};

class CORE_API FileReader
{
public:
	FileReader(const char* _path, Allocator* _allocator = nullptr);
	~FileReader();

	bool load();
	u32 getContentSize() const;
	const void* getContent() const;
	const char* getPath() const;

	Allocator& allocator() const;

private:
	String m_path;
	void* m_content = nullptr;
	u32 m_contentSize = 0;
	bool m_isLoaded = false;
	Allocator* m_allocator = nullptr;
};

} // namespace yae