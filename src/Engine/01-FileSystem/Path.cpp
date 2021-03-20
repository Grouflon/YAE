#include "Path.h"

//#include <filesystem>

#include <00-String/StringTools.h>

namespace yae {

Path::Path()
{
	
}

Path::Path(const char* _path, bool _normalize)
	: m_path(_path)
{
	if (_normalize)
	{
		NormalizePath(m_path);
	}
}

void Path::NormalizePath(String& _path)
{
	_path.replace("\\", "/");
}

}


