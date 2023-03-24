#pragma once

#include <yae/types.h>

#include <yae/Date.h>

// interesting stuff: http://utf8everywhere.org/

namespace yae {

class Allocator;

namespace filesystem {

// Path management
YAE_API String normalizePath(const char* _path);
YAE_API String& normalizePath(String& _path);
YAE_API String getAbsolutePath(const char* _path);
YAE_API String getRelativePath(const char* _path, const char* _relativeTo);
YAE_API void setWorkingDirectory(const char* _path);
YAE_API String getWorkingDirectory();
YAE_API String getDirectory(const char* _path);
YAE_API String getFileName(const char* _path);
YAE_API String getFileNameWithoutExtension(const char* _path);
YAE_API String getExtension(const char* _path);

// Filesystem actions
YAE_API bool doesPathExists(const char* _path);
YAE_API bool deletePath(const char* _path);
YAE_API bool createDirectory(const char* _path);
YAE_API Date getFileLastWriteTime(const char* _path);

enum CopyMode
{
	CopyMode_SkipExisting,
	CopyMode_OverwriteExisting,
	CopyMode_OverwriteExistingIfOlder,
};
YAE_API bool copy(const char* _from, const char* _to, CopyMode _mode = CopyMode_SkipExisting);

} // namespace filesystem

class YAE_API FileHandle
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

class YAE_API FileReader
{
public:
	FileReader(const char* _path, Allocator* _allocator = nullptr);
	~FileReader();

	bool load();
	u32 getContentSize() const;
	const void* getContent() const;

	Allocator& allocator() const;

private:
	String m_path;
	void* m_content = nullptr;
	u32 m_contentSize = 0;
	bool m_isLoaded = false;
	Allocator* m_allocator = nullptr;
};

} // namespace yae