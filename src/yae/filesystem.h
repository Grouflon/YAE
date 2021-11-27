#pragma once

#include <yae/types.h>

// interesting stuff: http://utf8everywhere.org/

namespace yae {

class Allocator;

namespace filesystem {

// Path management
YAELIB_API String normalizePath(const char* _path);
YAELIB_API String& normalizePath(String& _path);
YAELIB_API String getDirectory(const char* _path);
YAELIB_API String getAbsolutePath(const char* _path);
YAELIB_API String getRelativePath(const char* _path, const char* _relativeTo);
YAELIB_API void setWorkingDirectory(const char* _path);
YAELIB_API String getWorkingDirectory();

// Filesystem actions
YAELIB_API bool deletePath(const char* _path);
YAELIB_API bool createDirectory(const char* _path);

enum CopyMode
{
	CopyMode_SkipExisting,
	CopyMode_OverwriteExisting,
	CopyMode_OverwriteExistingIfOlder,
};
YAELIB_API bool copy(const char* _from, const char* _to, CopyMode _mode = CopyMode_SkipExisting);

} // namespace filesystem

class YAELIB_API FileHandle
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

} // namespace yae