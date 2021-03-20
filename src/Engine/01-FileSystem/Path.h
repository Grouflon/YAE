#pragma once

#include <export.h>

#include <00-String/String.h>

// interesting stuff: http://utf8everywhere.org/

namespace yae {

struct YAELIB_API Path
{
	Path();
	Path(const char* _path, bool _normalize = true);

	const String& getString() const { return m_path; }
	const char* c_str() const { return m_path.c_str(); }

	static void NormalizePath(String& _path);

private:
	String m_path;
};

}
