#include "ShaderCompiler.h"

#if YAE_USE_SHADERCOMPILER
#include <shaderc/shaderc.h>
#endif

namespace yae {

#if YAE_USE_SHADERCOMPILER
namespace { // anonymous

shaderc_shader_kind shaderTypeToShadercType(ShaderType _type)
{
	switch(_type)
	{
		case ShaderType::VERTEX: return shaderc_vertex_shader;
		case ShaderType::GEOMETRY: return shaderc_geometry_shader;
		case ShaderType::FRAGMENT: return shaderc_fragment_shader;

		case ShaderType::UNDEFINED: return shaderc_glsl_infer_from_source;
	}
	return shaderc_glsl_infer_from_source;
}

} // anonymous

void ShaderCompiler::init()
{
	YAE_CAPTURE_FUNCTION();

	m_compiler = shaderc_compiler_initialize();
	YAE_ASSERT(m_compiler != nullptr);
}


void ShaderCompiler::shutdown()
{
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(m_compiler != nullptr);
	shaderc_compiler_release((shaderc_compiler_t)m_compiler);
	m_compiler = nullptr;
}


ShaderCompilationResult ShaderCompiler::compile(const char* _source, size_t _sourceSize, const ShaderCompilationParams& _params)
{
	YAE_CAPTURE_FUNCTION();

	if (_source == nullptr || _sourceSize == 0)
	{
		YAE_ERROR("Can't compile empty source");
		return nullptr;
	}

	shaderc_compilation_result_t result;
	shaderc_compile_options_t options = shaderc_compile_options_initialize();
	YAE_ASSERT(options != nullptr);

	for(u16 i = 0; i < _params.defineCount; ++i)
	{
		const char* name = _params.defines[i];
		size_t nameLength = strlen(name);
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
		(shaderc_compiler_t)m_compiler, 
		_source,
		_sourceSize,
		shaderTypeToShadercType(_params.type),
		_params.shaderName,
		_params.entryPoint,
		options
	);

	shaderc_compile_options_release(options);
	YAE_ASSERT(result != nullptr);

	shaderc_compilation_status status = shaderc_result_get_compilation_status(result);
	if (status != shaderc_compilation_status_success)
	{
		size_t numWarnings = shaderc_result_get_num_warnings(result);
		size_t numErrors = shaderc_result_get_num_errors(result);
		if (numErrors > 0)
		{
			YAE_ERRORF("%s", shaderc_result_get_error_message(result));
		}
		else if (numWarnings > 0)
		{
			YAE_WARNINGF("%s", shaderc_result_get_error_message(result));
		}
	}

	return result;
}


bool ShaderCompiler::getResultData(ShaderCompilationResult _result, const void*& _outData, size_t& _outDataLength)
{
	shaderc_compilation_result_t result = (shaderc_compilation_result_t) _result;
	shaderc_compilation_status status = shaderc_result_get_compilation_status(result);
	if (status == shaderc_compilation_status_success)
	{
		_outData = shaderc_result_get_bytes(result);
		_outDataLength = shaderc_result_get_length(result);
		return true;
	}
	return false;
}


void ShaderCompiler::releaseResult(ShaderCompilationResult _result)
{
	YAE_ASSERT(_result != nullptr);
	shaderc_result_release((shaderc_compilation_result_t)_result);
}

#else

void ShaderCompiler::init() {}
void ShaderCompiler::shutdown() {}
bool compile(void* _source, size_t _sourceSize, ShaderCompilationResult& _outResult, const ShaderCompilationParams& _params = ShaderCompilationParams()) { return false; }
bool getResultData(ShaderCompilationResult _result, void*& _outData, size_t& _outDataLength) { return false; }
void releaseResult(ShaderCompilationResult _result) {}

#endif

} // namespace yae
