#pragma once

#include <cstdio>

#include <yae/types.h>
#include <yae/string.h>

// interesting stuff: http://utf8everywhere.org/

namespace yae {

class Allocator;

struct YAELIB_API Path
{
	Path(Allocator* _allocator = nullptr);
	Path(const char* _path, bool _normalize = true, Allocator* _allocator = nullptr);
	~Path();

	const String& getString() const { return m_path; }
	const char* c_str() const { return m_path.c_str(); }

	static void NormalizePath(String& _path);

private:
	String m_path;
};



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