#include "ShaderProgram.h"

#include <yae/resources/Shader.h>
#include <yae/rendering/Renderer.h>
#include <yae/ResourceManager.h>

MIRROR_CLASS(yae::ShaderProgram)
(
	MIRROR_PARENT(yae::Resource);

	MIRROR_MEMBER(m_shaderStages);
);

namespace yae {

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
	for (u32 i = 0; i < _shaderCount; ++i)
	{
		m_shaderStages[i] = _shaders[i];
	}
}

const DataArray<ResourcePtr<Shader>>& ShaderProgram::getShaderStages()
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
	for(ResourcePtr<Shader> shaderPtr : m_shaderStages)
	{
		Shader* shader = shaderPtr.get();
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
	for(ResourcePtr<Shader> shaderPtr : m_shaderStages)
	{
		Shader* shader = shaderPtr.get();
		handles.push_back(shader->getShaderHandle());
		m_loadedShaders.push_back(shader);
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
	for(Shader* shader : m_loadedShaders)
	{
		resourceManager().removeDependency(shader, this);
		shader->unload();
	}
	m_loadedShaders.clear();
}

} // namespace yae
