#include "ShaderResource.h"

#include <yae/filesystem.h>
#include <yae/platform.h>
#include <yae/program.h>
#include <yae/resources/FileResource.h>
#include <yae/Renderer.h>
#include <yae/ShaderCompiler.h>

//#include <shaderc/shaderc.h>

namespace yae {

namespace { // anonymous

/*
String getCompiledShaderDirectory()
{
	String compiledShaderDirectory = String(program().getIntermediateDirectory(), &scratchAllocator()) + "shaders/";
	return filesystem::normalizePath(compiledShaderDirectory);
}


void ensureCompiledShaderDirectory()
{
	yae::filesystem::createDirectory(getCompiledShaderDirectory().c_str());
}
*/


const char* shaderTypeToString(ShaderType _type)
{
	switch(_type)
	{
		case ShaderType::UNDEFINED: return "undefined";
		case ShaderType::VERTEX: return "vertex";
		case ShaderType::GEOMETRY: return "geometry";
		case ShaderType::FRAGMENT: return "fragment";
	}
	return "";
}


/*shaderc_shader_kind shaderTypeToShadercType(ShaderType _type)
{
	switch(_type)
	{
		case SHADERTYPE_VERTEX: return shaderc_vertex_shader;
		case SHADERTYPE_GEOMETRY: return shaderc_geometry_shader;
		case SHADERTYPE_FRAGMENT: return shaderc_fragment_shader;
	}
	return shaderc_glsl_infer_from_source;
}*/


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

	/*ensureCompiledShaderDirectory();

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

		if (program().shaderCompiler() != nullptr)
		{
			DataArray<const char*> defines(&scratchAllocator());
			for (const String& define : m_defines)
			{
				defines.push_back(define.c_str());
			}

			ShaderCompilationParams params;
			params.type = m_shaderType;
			params.shaderName = m_path.c_str();
			params.entryPoint = m_entryPoint.c_str();
			params.defines = defines.data();
			params.defineCount = defines.size();

			ShaderCompilationResult result = program().shaderCompiler()->compile(
				(const char*)content,
				contentSize,
				params
			);

			const void* compiledBinary = nullptr;
			size_t compiledBinarySize = 0;
			if (!program().shaderCompiler()->getResultData(result, compiledBinary, compiledBinarySize))
			{
				program().shaderCompiler()->releaseResult(result);
				_log(RESOURCELOGTYPE_ERROR, "Failed to compile shader.");
				return;
			}

			{
				YAE_CAPTURE_SCOPE("write_compiled_file");

				FileHandle compiledFileHandle(compiledShaderPath.c_str());
		    	if (!compiledFileHandle.open(FileHandle::OPENMODE_WRITE))
		    	{
					_log(RESOURCELOGTYPE_ERROR, string::format("Could not open \"%s\" for write.", compiledShaderPath.c_str()).c_str());
					program().shaderCompiler()->releaseResult(result);
					return;
		    	}
		    	compiledFileHandle.write(compiledBinary, compiledBinarySize);
		    	compiledFileHandle.close();	
			}

			if (!renderer().createShader(m_shaderType, compiledBinary, compiledBinarySize, m_shaderHandle))
			{
				_log(RESOURCELOGTYPE_ERROR, "Failed to create shader on the renderer.");
			}
			program().shaderCompiler()->releaseResult(result);
		}
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
    	
    	if (!renderer().createShader(compiledFile->getContent(), compiledFile->getContentSize(), m_shaderHandle))
    	{
			_log(RESOURCELOGTYPE_ERROR, "Failed to create shader on the renderer.");
		}
		compiledFile->releaseUnuse();
	}
	*/
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
