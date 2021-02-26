#pragma once

#include <string>

// interesting stuff: http://utf8everywhere.org/

namespace yae {

struct Path
{
	Path(const char* _path, bool _normalize = true);

	const std::string& getString() const { return m_path; }
	const char* getCString() const { return m_path.c_str(); }

	static void NormalizePath(std::string& _path);

	static const Path WORKING_DIRECTORY;

private:

	std::string m_path;
};

}
