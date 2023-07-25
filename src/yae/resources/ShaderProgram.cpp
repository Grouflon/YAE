#include "ShaderProgram.h"

#include <yae/resources/Shader.h>
#include <yae/rendering/Renderer.h>
#include <yae/ResourceManager.h>

namespace yae {

MIRROR_CLASS_DEFINITION(ShaderProgram);

ShaderProgram::ShaderProgram()
{

}

ShaderProgram::~ShaderProgram()
{

}

void ShaderProgram::setShaderStages(Shader** _shaders, u32 _shaderCount)
{
	YAE_ASSERT(m_shaderProgramHandle == 0);

	m_shaderStages.resize(_shaderCount);
	memcpy(m_shaderStages.data(), _shaders, sizeof(*_shaders) * _shaderCount);
}

const DataArray<Shader*>& ShaderProgram::getShaderStages()
{
	return m_shaderStages;
}

void ShaderProgram::setPrimitiveMode(PrimitiveMode _primitiveMode)
{
	YAE_ASSERT(m_shaderProgramHandle == 0);

	m_primitiveMode = _primitiveMode;
}

PrimitiveMode ShaderProgram::getPrimitiveMode() const
{
	return m_primitiveMode;
}

ShaderProgramHandle ShaderProgram::getShaderProgramHandle() const
{
	return m_shaderProgramHandle;
}

void ShaderProgram::_doLoad()
{
	YAE_CAPTURE_FUNCTION();

	// Load shaders
	bool allShadersLoaded = true;
	for(Shader* shader : m_shaderStages)
	{
		shader->load();
		resourceManager().addDependency(shader, this);
		allShadersLoaded = allShadersLoaded && shader->isLoaded();
	}

	if (!allShadersLoaded)
	{
		_log(RESOURCELOGTYPE_ERROR, "Can't create shader program, shaders aren't loaded properly.");
		return;
	}

	DataArray<ShaderHandle> handles(&scratchAllocator());
	handles.reserve(m_shaderStages.size());
	for(Shader* shader : m_shaderStages)
	{
		handles.push_back(shader->getShaderHandle());
	}

	if (!renderer().createShaderProgram(handles.data(), handles.size(), m_shaderProgramHandle))
	{
		_log(RESOURCELOGTYPE_ERROR, "Failed to create shader program on the renderer.");
		return;
	}
}

void ShaderProgram::_doUnload()
{
	YAE_CAPTURE_FUNCTION();

	renderer().destroyShaderProgram(m_shaderProgramHandle);
	m_shaderProgramHandle = 0;

	// Release shaders
	for(Shader* shader : m_shaderStages)
	{
		resourceManager().removeDependency(shader, this);
		shader->unload();
	}
}

} // namespace yae
