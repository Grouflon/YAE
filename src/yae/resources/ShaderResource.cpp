#include "ShaderResource.h"

#include <yae/filesystem.h>
#include <yae/platform.h>
#include <yae/program.h>
#include <yae/resources/FileResource.h>
#include <yae/vulkan/VulkanRenderer.h>

#include <shaderc/shaderc.h>

namespace yae {

namespace { // anonymous

String getCompiledShaderDirectory()
{
	String compiledShaderDirectory = String(program().getIntermediateDirectory(), &scratchAllocator()) + "shaders/";
	return filesystem::normalizePath(compiledShaderDirectory);
}


void ensureCompiledShaderDirectory()
{
	yae::filesystem::createDirectory(getCompiledShaderDirectory().c_str());
}


const char* shaderTypeToString(ShaderType _type)
{
	switch(_type)
	{
		case SHADERTYPE_VERTEX: return "vertex";
		case SHADERTYPE_FRAGMENT: return "fragment";
	}
	return "";
}


shaderc_shader_kind shaderTypeToShadercType(ShaderType _type)
{
	switch(_type)
	{
		case SHADERTYPE_VERTEX: return shaderc_vertex_shader;
		case SHADERTYPE_FRAGMENT: return shaderc_fragment_shader;
	}
	return shaderc_glsl_infer_from_source;
}


String buildShaderName(const char* _path, ShaderType _type, const char* _entryPoint)
{
	return string::format("%s_%s_%s", filesystem::normalizePath(_path).c_str(), shaderTypeToString(_type), _entryPoint);
}

} // anonymous


MIRROR_CLASS_DEFINITION(ShaderResource);

ShaderResource::ShaderResource(const char* _path, ShaderType _type, const char* _entryPoint)
	: Resource(buildShaderName(_path, _type, _entryPoint).c_str())
	, m_path(_path)
	, m_shaderType(_type)
	, m_entryPoint(_entryPoint)
{
}


ShaderResource::~ShaderResource()
{
}


void ShaderResource::_doLoad()
{
	YAE_CAPTURE_FUNCTION();

	ensureCompiledShaderDirectory();

	if (!filesystem::doesPathExists(m_path.c_str()))
	{
		_log(RESOURCELOGTYPE_ERROR, "Could not open file.");
		return;
	}

	String compiledShaderFileName(string::format("%s_%s_%s", filesystem::getFileNameWithoutExtension(m_path.c_str()).c_str(), shaderTypeToString(m_shaderType), m_entryPoint.c_str()) + ".spv", &scratchAllocator());
	String compiledShaderPath(getCompiledShaderDirectory() + compiledShaderFileName, &scratchAllocator());

	u64 sourceFileTime = platform::getFileLastWriteTime(m_path.c_str());
	u64 compiledFileTime = platform::getFileLastWriteTime(compiledShaderPath.c_str());

	if (sourceFileTime > compiledFileTime)
	{
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
		
		// @TODO: the compiler and options takes a long time to be initialized and should be shared. Probably through the program for now
		shaderc_compiler_t shaderCompiler;
		{
			YAE_CAPTURE_SCOPE("init_compiler");

			shaderCompiler = shaderc_compiler_initialize();
			YAE_ASSERT(shaderCompiler != nullptr);
		}

		shaderc_compile_options_t compileOptions;
		{
			YAE_CAPTURE_SCOPE("init_compile_options");

			compileOptions = shaderc_compile_options_initialize();
			YAE_ASSERT(compileOptions != nullptr);
		}

		shaderc_compilation_result_t result;

		{
			YAE_CAPTURE_SCOPE("compile_shader");

			result = shaderc_compile_into_spv(
				shaderCompiler, 
				(const char *)content,
				contentSize,
				shaderTypeToShadercType(m_shaderType),
				m_path.c_str(),
				m_entryPoint.c_str(),
				compileOptions
			);
		}

		scratchAllocator().deallocate(content);
		shaderc_compile_options_release(compileOptions);
	    shaderc_compiler_release(shaderCompiler);

		if (result == nullptr)
		{
			_log(RESOURCELOGTYPE_ERROR, "Compilation process failed.");
			return;
		}

		shaderc_compilation_status status = shaderc_result_get_compilation_status(result);
		if (status != shaderc_compilation_status_success)
		{
			size_t numWarnings = shaderc_result_get_num_warnings(result);
			size_t numErrors = shaderc_result_get_num_errors(result);
			if (numErrors > 0)
			{
				_log(RESOURCELOGTYPE_ERROR, shaderc_result_get_error_message(result));
			}
			else if (numWarnings > 0)
			{
				_log(RESOURCELOGTYPE_WARNING, shaderc_result_get_error_message(result));
			}
			shaderc_result_release(result);
	    	return;
		}

		{
			YAE_CAPTURE_SCOPE("write_compiled_file");

			FileHandle compiledFileHandle(compiledShaderPath.c_str());
	    	if (!compiledFileHandle.open(FileHandle::OPENMODE_WRITE))
	    	{
				_log(RESOURCELOGTYPE_ERROR, string::format("Could not open \"%s\" for write.", compiledShaderPath.c_str()).c_str());
				shaderc_result_release(result);
				return;
	    	}
	    	compiledFileHandle.write(shaderc_result_get_bytes(result), shaderc_result_get_length(result));
	    	compiledFileHandle.close();	
		}

		renderer().createShader(shaderc_result_get_bytes(result), shaderc_result_get_length(result), m_shaderHandle);
		shaderc_result_release(result);
	}
	else
	{
		FileResource* compiledFile;
		{
			YAE_CAPTURE_SCOPE("read_compiled_file");

			compiledFile = findOrCreateResource<FileResource>(compiledShaderPath.c_str());
			compiledFile->useLoad();
			if (!compiledFile->isLoaded())
			{
				_log(RESOURCELOGTYPE_ERROR, string::format("Could not open \"%s\".", compiledShaderPath.c_str()).c_str());
				compiledFile->releaseUnuse();
				return;
			}	
		}
    	
    	renderer().createShader(compiledFile->getContent(), compiledFile->getContentSize(), m_shaderHandle);
		compiledFile->releaseUnuse();
	}
}


void ShaderResource::_doUnload()
{
	YAE_CAPTURE_FUNCTION();

	renderer().destroyShader(m_shaderHandle);
}


ResourceID ResourceIDGetter<ShaderResource>::GetId(const char* _path, ShaderType _type, const char* _entryPoint)
{
	return ResourceID(buildShaderName(_path, _type, _entryPoint).c_str());
}

} // namespace yae
