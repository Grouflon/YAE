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
		case SHADERTYPE_GEOMETRY: return "geometry";
		case SHADERTYPE_FRAGMENT: return "fragment";
	}
	return "";
}


shaderc_shader_kind shaderTypeToShadercType(ShaderType _type)
{
	switch(_type)
	{
		case SHADERTYPE_VERTEX: return shaderc_vertex_shader;
		case SHADERTYPE_GEOMETRY: return shaderc_geometry_shader;
		case SHADERTYPE_FRAGMENT: return shaderc_fragment_shader;
	}
	return shaderc_glsl_infer_from_source;
}


String buildShaderName(const char* _path, ShaderType _type, const char* _entryPoint, const char** _defines, size_t _defineCount)
{
	String definesString(&scratchAllocator());
	for(size_t i = 0; i < _defineCount; ++i)
	{
		definesString += "#";
		definesString += _defines[i];
	}

	return string::format("%s_%s_%s_%s", filesystem::normalizePath(_path).c_str(), shaderTypeToString(_type), _entryPoint, definesString.c_str());
}

} // anonymous


MIRROR_CLASS_DEFINITION(ShaderResource);

ShaderResource::ShaderResource(const char* _path, ShaderType _type, const char* _entryPoint, const char** _defines, size_t _defineCount)
	: Resource(buildShaderName(_path, _type, _entryPoint, _defines, _defineCount).c_str())
	, m_shaderType(_type)
	, m_path(_path)
	, m_entryPoint(_entryPoint)
{
	for (size_t i = 0; i < _defineCount; ++i)
	{
		m_defines.push_back(_defines[i]);
	}
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

	String definesString(&scratchAllocator());
	for(const String& define : m_defines)
	{
		definesString += "#";
		definesString += define;
	}
	String compiledShaderFileName(string::format("%s_%s_%s_%s", filesystem::getFileNameWithoutExtension(m_path.c_str()).c_str(), shaderTypeToString(m_shaderType), m_entryPoint.c_str(), definesString.c_str()) + ".spv", &scratchAllocator());
	String compiledShaderPath(getCompiledShaderDirectory() + compiledShaderFileName, &scratchAllocator());

	Date sourceFileTime = filesystem::getFileLastWriteTime(m_path.c_str());
	Date compiledFileTime = filesystem::getFileLastWriteTime(compiledShaderPath.c_str());

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

		shaderc_compilation_result_t result;
		{
			YAE_CAPTURE_SCOPE("compile_shader");

			shaderc_compile_options_t options = shaderc_compile_options_initialize();
			YAE_ASSERT(options != nullptr);

			for(const String& define : m_defines)
			{
				const char* name = define.c_str();
				size_t nameLength = define.size();
				const char* value = "";
				size_t valueLength = 0;

				const char* equal = strchr(name, '=');
				if (equal != nullptr)
				{
					value = equal + 1;
					valueLength = nameLength - size_t(equal - name) - 1;
					nameLength = size_t(equal - name);
				}
				shaderc_compile_options_add_macro_definition(options, name, nameLength, value, valueLength);
			}

			result = shaderc_compile_into_spv(
				program().shaderCompiler(), 
				(const char *)content,
				contentSize,
				shaderTypeToShadercType(m_shaderType),
				m_path.c_str(),
				m_entryPoint.c_str(),
				options
			);

			shaderc_compile_options_release(options);
		}

		scratchAllocator().deallocate(content);

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

		if (compiledFile->getContentSize() == 0)
		{
			_log(RESOURCELOGTYPE_ERROR, string::format("Compiled file \"%s\" is empty.", compiledShaderPath.c_str()).c_str());
			compiledFile->releaseUnuse();
			return;
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


ResourceID ResourceIDGetter<ShaderResource>::GetId(const char* _path, ShaderType _type, const char* _entryPoint, const char** _defines, size_t _defineCount)
{
	return ResourceID(buildShaderName(_path, _type, _entryPoint, _defines, _defineCount).c_str());
}

} // namespace yae
