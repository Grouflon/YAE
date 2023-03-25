#include "Shader.h"

#include <yae/rendering/Renderer.h>

namespace yae {

MIRROR_CLASS_DEFINITION(Shader);

Shader::Shader()
{

}

Shader::~Shader()
{

}

void Shader::setShaderData(const void* _data, u32 _dataSize)
{
	YAE_ASSERT(m_shaderHandle == 0);

	defaultAllocator().deallocate(m_data);
	m_data = defaultAllocator().allocate(_dataSize);
	memcpy(m_data, _data, _dataSize);
	m_dataSize = _dataSize;
}

void Shader::getShaderData(const void*& _data, u32& _dataSize) const
{
	_data = m_data;
	_dataSize = m_dataSize;
}

void Shader::setShaderType(ShaderType _type)
{
	YAE_ASSERT(m_shaderHandle == 0);

	m_shaderType = _type;
}

ShaderType Shader::getShaderType() const
{
	return m_shaderType;
}

const ShaderHandle& Shader::getShaderHandle()
{
	return m_shaderHandle;
}

void Shader::_doLoad()
{
	YAE_CAPTURE_FUNCTION();
	YAE_ASSERT(m_shaderHandle == 0);

	if (!renderer().createShader(m_shaderType, (const char*)m_data, m_dataSize, m_shaderHandle))
	{
		_log(RESOURCELOGTYPE_ERROR, "Failed to create shader on the renderer.");
		return;
	}
}

void Shader::_doUnload()
{
	YAE_CAPTURE_FUNCTION();

	renderer().destroyShader(m_shaderHandle);
	m_shaderHandle = 0;
}

} // namespace yae
