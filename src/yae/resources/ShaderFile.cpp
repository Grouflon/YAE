#include "ShaderFile.h"

#include <yae/filesystem.h>
#include <yae/rendering/Renderer.h>
#include <yae/string.h>

namespace yae {

MIRROR_CLASS_DEFINITION(ShaderFile);

ShaderFile::ShaderFile()
{
}


ShaderFile::~ShaderFile()
{
}

void ShaderFile::setPath(const char* _path)
{
	YAE_ASSERT(!isLoaded());
	m_path = filesystem::normalizePath(_path);
}

const char* ShaderFile::getPath() const
{
	return m_path.c_str();
}

void ShaderFile::setShaderType(ShaderType _type)
{
	YAE_ASSERT(!isLoaded());
	m_shaderType = _type;
}

ShaderType ShaderFile::getShaderType() const
{
	return m_shaderType;
}

const ShaderHandle& ShaderFile::getShaderHandle()
{
	return m_shaderHandle;
}

void ShaderFile::_doLoad()
{
	YAE_CAPTURE_FUNCTION();

	FileReader reader(m_path.c_str());
	if (!reader.load())
	{
		_log(RESOURCELOGTYPE_ERROR, string::format("Could not load file \"%s\".", m_path.c_str()).c_str());
		return;
	}

	if (!renderer().createShader(m_shaderType, (const char*)reader.getContent(), reader.getContentSize(), m_shaderHandle))
	{
		_log(RESOURCELOGTYPE_ERROR, "Failed to create shader on the renderer.");
	}
}

void ShaderFile::_doUnload()
{
	YAE_CAPTURE_FUNCTION();

	renderer().destroyShader(m_shaderHandle);
}

} // namespace yae
