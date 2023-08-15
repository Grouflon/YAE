#include "ShaderFile.h"

#include <yae/filesystem.h>
#include <yae/ResourceManager.h>
#include <yae/string.h>

MIRROR_CLASS(yae::ShaderFile)
(
	MIRROR_PARENT(yae::Shader);

	MIRROR_MEMBER(m_path);
);

namespace yae {

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

void ShaderFile::_doLoad()
{
	YAE_CAPTURE_FUNCTION();

	resourceManager().startReloadOnFileChanged(m_path.c_str(), this);

	FileReader reader(m_path.c_str(), &scratchAllocator());
	if (!reader.load())
	{
		_log(RESOURCELOGTYPE_ERROR, string::format("Could not load file \"%s\".", m_path.c_str()).c_str());
	}

	setShaderData(reader.getContent(), reader.getContentSize());

	Shader::_doLoad();
}

void ShaderFile::_doUnload()
{
	YAE_CAPTURE_FUNCTION();

	Shader::_doUnload();

	setShaderData(nullptr, 0);

	resourceManager().stopReloadOnFileChanged(m_path.c_str(), this);
}

} // namespace yae
