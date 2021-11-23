#include "MeshResource.h"

#include <yae/filesystem.h>

namespace yae {

MIRROR_CLASS_DEFINITION(MeshResource);

MeshResource::MeshResource(const char* _path)
	: Resource(filesystem::normalizePath(_path).c_str())
{

}


MeshResource::~MeshResource()
{

}


Resource::Error MeshResource::_doLoad(String& _outErrorDescription)
{
	return ERROR_NONE;
}


void MeshResource::_doUnload()
{

}

} // namespace yae
