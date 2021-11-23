#pragma once

#include <cstdio>

#include <yae/types.h>
#include <yae/inline_string.h>

// interesting stuff: http://utf8everywhere.org/

namespace yae {

class Allocator;

namespace filesystem {

YAELIB_API String normalizePath(const char* _path);
YAELIB_API String& normalizePath(String& _path);
YAELIB_API String getDirectory(const char* _path);

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
	String256	m_path;
	OpenMode	m_openMode;
	std::FILE*	m_file;
};

} // namespace yae