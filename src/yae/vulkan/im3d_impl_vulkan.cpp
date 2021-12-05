#include "im3d_impl_vulkan.h"

#include <yae/resources/ShaderResource.h>

const char* SHADER_PATH = "./data/shaders/im3d.glsl";

namespace yae {

im3d_Instance* im3d_Init()
{
	im3d_Instance* instance = toolAllocator().create<im3d_Instance>();
	*instance = {};
	{
		const char* defines[] = {
			"VERTEX_SHADER",
			"LINES"
		};
		instance->vertexShader = findOrCreateResource<ShaderResource>(SHADER_PATH, SHADERTYPE_VERTEX, "main", defines, countof(defines));
		YAE_ASSERT(instance->vertexShader);
		instance->vertexShader->useLoad();
	}

	{
		const char* defines[] = {
			"GEOMETRY_SHADER",
			"LINES"
		};
		instance->geometryShader = findOrCreateResource<ShaderResource>(SHADER_PATH, SHADERTYPE_GEOMETRY, "main", defines, countof(defines));
		YAE_ASSERT(instance->geometryShader);
		instance->geometryShader->useLoad();
	}

	{
		const char* defines[] = {
			"FRAGMENT_SHADER",
			"LINES"
		};
		instance->fragmentShader = findOrCreateResource<ShaderResource>(SHADER_PATH, SHADERTYPE_FRAGMENT, "main", defines, countof(defines));
		YAE_ASSERT(instance->fragmentShader);
		instance->fragmentShader->useLoad();
	}

	return instance;
}


void im3d_Shutdown(im3d_Instance* _instance)
{
	_instance->fragmentShader->releaseUnuse();
	_instance->fragmentShader = nullptr;

	_instance->geometryShader->releaseUnuse();
	_instance->geometryShader = nullptr;

	_instance->vertexShader->releaseUnuse();
	_instance->vertexShader = nullptr;

	toolAllocator().destroy(_instance);
}


void im3d_NewFrame(im3d_Instance* _instance)
{

}


void im3d_EndFrame(im3d_Instance* _instance)
{

}

} // namespace yae
