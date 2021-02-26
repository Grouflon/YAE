#include "Path.h"

#include <filesystem>

#include <01-String/StringTools.h>

namespace yae {

Path::Path(const char* _path, bool _normalize)
	: m_path(_path)
{
	if (_normalize)
	{
		NormalizePath(m_path);
	}
}

void Path::NormalizePath(std::string& _path)
{
	ReplaceInline(_path, "\\", "/");
}

const Path Path::WORKING_DIRECTORY = Path(Narrow(std::filesystem::current_path().c_str()).c_str(), true);

}


