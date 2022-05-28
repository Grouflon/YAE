#include "ShaderResource.h"

#include <yae/filesystem.h>
#include <yae/platform.h>
#include <yae/program.h>
#include <yae/resources/FileResource.h>
#include <yae/Renderer.h>

namespace yae {

MIRROR_CLASS_DEFINITION(ShaderResource);

ShaderResource::ShaderResource(const char* _path, ShaderType _type)
	: Resource(_path)
	, m_shaderType(_type)
	, m_path(_path)
{
}


ShaderResource::~ShaderResource()
{
}


void ShaderResource::_doLoad()
{
	YAE_CAPTURE_FUNCTION();

	FileHandle file(m_path.c_str());
	size_t contentSize = 0;
	void* content = nullptr;
	{
		YAE_CAPTURE_SCOPE("read_shader_source");

		if (!file.open(FileHandle::OPENMODE_READ))
		{
			_log(RESOURCELOGTYPE_ERROR, string::format("Could not open file \"%s\".", m_path.c_str()).c_str());
			return;
		}

		contentSize = file.getSize();
		content = scratchAllocator().allocate(contentSize);
		file.read(content, contentSize);
	}

	if (!renderer().createShader(m_shaderType, (const char*)content, contentSize, m_shaderHandle))
	{
		_log(RESOURCELOGTYPE_ERROR, "Failed to create shader on the renderer.");
	}
	scratchAllocator().deallocate(content);
}


void ShaderResource::_doUnload()
{
	YAE_CAPTURE_FUNCTION();

	renderer().destroyShader(m_shaderHandle);
}


ResourceID ResourceIDGetter<ShaderResource>::GetId(const char* _path, ShaderType _type)
{
	return ResourceID(_path);
}

} // namespace yae
