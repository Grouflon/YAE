#include "OpenGLRenderer.h"

#include <yae/program.h>
#include <yae/resources/ShaderResource.h>
#include <yae/resources/FontResource.h>
#include <yae/resources/FileResource.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#else
#include <GL/gl3w.h>
#endif
#include <imgui/backends/imgui_impl_glfw.h>
#include <im3d/im3d.h>
#include <GLFW/glfw3.h>

#if YAE_PLATFORM_WEB == 0
const int OPENGL_VERSION_MAJOR = 4;
const int OPENGL_VERSION_MINOR = 3;
const char* OPENGL_SHADER_VERSION = "#version 330";
#define YAE_OPENGL_ES 0
#else
const int OPENGL_VERSION_MAJOR = 3;
const int OPENGL_VERSION_MINOR = 0;
const char* OPENGL_SHADER_VERSION = "#version 300 es";
#define YAE_OPENGL_ES 1
#endif


const char* glErrorToString(GLint _errorCode)
{
	switch (_errorCode)
	{
		case GL_NO_ERROR: return "GL_NO_ERROR";
		case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
		case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
		default: return "Unknown error";
	}
}


#define YAE_GL_VERIFY(_instruction) { _instruction; GLint ___error = glGetError(); YAE_ASSERT_MSGF(___error == GL_NO_ERROR, "GL Error %s(0x%04x) -> " #_instruction, glErrorToString(___error), ___error); }

namespace yae {

struct GLMeshHandle
{
	GLuint vertexBufferId;
	GLuint indexBufferId;
	GLuint indicesCount;
};

void glDebugCallback(GLenum _source, GLenum _type, GLuint _id, GLenum _severity, GLsizei _length, const GLchar* _msg, const void* _data)
{
	if (_id == 0x20071) return; // Message about buffer usage hints when calling glBufferData

	switch(_severity)
	{
		case GL_DEBUG_SEVERITY_HIGH: YAE_ERRORF_CAT("renderer", "gldebug 0x%04x:0x%04x:0x%04x: %s", _source, _type, _id, _msg); break;
		case GL_DEBUG_SEVERITY_MEDIUM: YAE_WARNINGF_CAT("renderer", "gldebug 0x%04x:0x%04x:0x%04x: %s", _source, _type, _id, _msg); break;
		default: YAE_VERBOSEF_CAT("renderer", "gldebug 0x%04x:0x%04x:0x%04x: %s", _source, _type, _id, _msg); break;
	}
}

void OpenGLRenderer::hintWindow()
{
#if YAE_OPENGL_ES
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#else
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
#endif

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
}

bool OpenGLRenderer::init(GLFWwindow* _window)
{
	glfwMakeContextCurrent(_window);
    glfwSwapInterval(1); // Enable vsync

    /*
    YAE_LOGF("GL_ARB_gl_spirv: %d", glfwExtensionSupported("GL_ARB_gl_spirv"));
    YAE_LOGF("GL_NV_shader_noperspective_interpolation: %d", glfwExtensionSupported("GL_NV_shader_noperspective_interpolation"));
    YAE_LOGF("ANGLE_instanced_arrays: %d", glfwExtensionSupported("ANGLE_instanced_arrays"));
    YAE_LOGF("EXT_blend_minmax: %d", glfwExtensionSupported("EXT_blend_minmax"));
    YAE_LOGF("EXT_frag_depth: %d", glfwExtensionSupported("EXT_frag_depth"));
    YAE_LOGF("EXT_shader_texture_lod: %d", glfwExtensionSupported("EXT_shader_texture_lod"));
    YAE_LOGF("EXT_texture_filter_anisotropic: %d", glfwExtensionSupported("EXT_texture_filter_anisotropic"));
    YAE_LOGF("OES_element_index_uint: %d", glfwExtensionSupported("OES_element_index_uint"));
    YAE_LOGF("OES_standard_derivatives: %d", glfwExtensionSupported("OES_standard_derivatives"));
    YAE_LOGF("OES_texture_float: %d", glfwExtensionSupported("OES_texture_float"));
    YAE_LOGF("OES_texture_float_linear: %d", glfwExtensionSupported("OES_texture_float_linear"));
    YAE_LOGF("OES_texture_half_float: %d", glfwExtensionSupported("OES_texture_half_float"));
    YAE_LOGF("OES_texture_half_float_linear: %d", glfwExtensionSupported("OES_texture_half_float_linear"));
    YAE_LOGF("OES_vertex_array_object: %d", glfwExtensionSupported("OES_vertex_array_object"));
    YAE_LOGF("WEBGL_compressed_texture_s3tc: %d", glfwExtensionSupported("WEBGL_compressed_texture_s3tc"));
    YAE_LOGF("WEBGL_debug_renderer_info: %d", glfwExtensionSupported("WEBGL_debug_renderer_info"));
    YAE_LOGF("WEBGL_debug_shaders: %d", glfwExtensionSupported("WEBGL_debug_shaders"));
    YAE_LOGF("WEBGL_depth_texture: %d", glfwExtensionSupported("WEBGL_depth_texture"));
    YAE_LOGF("WEBGL_draw_buffers: %d", glfwExtensionSupported("WEBGL_draw_buffers"));
    YAE_LOGF("WEBGL_lose_context: %d", glfwExtensionSupported("WEBGL_lose_context"));
	*/

#if YAE_PLATFORM_WEB == 0
	program().initGl3w();
    
	if (gl3wIsSupported(OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR) == false)
	{
		YAE_ERRORF_CAT("renderer", "Failed to initialize gl3w: Version %d.%d not supported", OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR);
		return false;
	}

	GLint v;
	glGetIntegerv(GL_CONTEXT_FLAGS, &v);
	if (v & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		YAE_VERBOSE_CAT("renderer", "OpenGL debug context present");
		glDebugMessageCallback(&glDebugCallback, nullptr);
	}
#endif

	const char* glVersion = (const char*)glGetString(GL_VERSION);
	YAE_LOGF_CAT("renderer", "OpenGL Version: \"%s\"", glVersion);

	YAE_GL_VERIFY(glGenVertexArrays(1, &m_vao));
	YAE_GL_VERIFY(glBindVertexArray(m_vao));

	GLuint buffers[2] = {} ; // 0 is vertices, 1 is indices
	YAE_GL_VERIFY(glGenBuffers(2, buffers));
	m_vertexBufferObject = buffers[0];
	m_indexBufferObject = buffers[1];

	YAE_GL_VERIFY(glBindBuffer(GL_ARRAY_BUFFER, (GLuint)m_vertexBufferObject));
	YAE_GL_VERIFY(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)m_indexBufferObject));

	int maxVertexAttribs;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
	YAE_ASSERT(maxVertexAttribs >= 3);

	YAE_GL_VERIFY(glEnableVertexAttribArray(0));
	YAE_GL_VERIFY(glEnableVertexAttribArray(1));
	YAE_GL_VERIFY(glEnableVertexAttribArray(2));
	YAE_GL_VERIFY(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)0));
	YAE_GL_VERIFY(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(sizeof(float)*3)));
	YAE_GL_VERIFY(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(sizeof(float)*6)));

	// Mesh shader
	{
		ShaderResource* vertexShader = findOrCreateResource<ShaderResource>("./data/shaders/shader.vert", ShaderType::VERTEX);
		vertexShader->useLoad();
		YAE_ASSERT(vertexShader->isLoaded());

		ShaderResource* fragmentShader = findOrCreateResource<ShaderResource>("./data/shaders/shader.frag", ShaderType::FRAGMENT);
		fragmentShader->useLoad();
		YAE_ASSERT(fragmentShader->isLoaded());

		ShaderHandle shaders[] =
		{
			vertexShader->getShaderHandle(),
			fragmentShader->getShaderHandle()
		};
		YAE_VERIFY(createShaderProgram(shaders, countof(shaders), m_shader));

		YAE_GL_VERIFY(glBindAttribLocation((GLuint)m_shader, 0, "inPosition"));
		YAE_GL_VERIFY(glBindAttribLocation((GLuint)m_shader, 1, "inColor"));
		YAE_GL_VERIFY(glBindAttribLocation((GLuint)m_shader, 2, "inTexCoord"));

		fragmentShader->releaseUnuse();
		vertexShader->releaseUnuse();
	}

	// Font shader
	{
		ShaderResource* vertexShader = findOrCreateResource<ShaderResource>("./data/shaders/font.vert", ShaderType::VERTEX);
		vertexShader->useLoad();
		YAE_ASSERT(vertexShader->isLoaded());

		ShaderResource* fragmentShader = findOrCreateResource<ShaderResource>("./data/shaders/font.frag", ShaderType::FRAGMENT);
		fragmentShader->useLoad();
		YAE_ASSERT(fragmentShader->isLoaded());

		ShaderHandle shaders[] =
		{
			vertexShader->getShaderHandle(),
			fragmentShader->getShaderHandle()
		};
		YAE_VERIFY(createShaderProgram(shaders, countof(shaders), m_fontShader));

		YAE_GL_VERIFY(glBindAttribLocation((GLuint)m_fontShader, 0, "inPosition"));
		YAE_GL_VERIFY(glBindAttribLocation((GLuint)m_fontShader, 1, "inColor"));
		YAE_GL_VERIFY(glBindAttribLocation((GLuint)m_fontShader, 2, "inTexCoord"));

		fragmentShader->releaseUnuse();
		vertexShader->releaseUnuse();
	}

	m_window = _window;
	return true;
}

void OpenGLRenderer::shutdown()
{
	destroyShaderProgram(m_fontShader);
	m_fontShader = nullptr;

	destroyShaderProgram(m_shader);
	m_shader = nullptr;

	GLuint buffers[2] = { m_vertexBufferObject, m_indexBufferObject};
	glDeleteBuffers(2, buffers);
	m_vertexBufferObject = 0;
	m_indexBufferObject = 0;

	glDeleteVertexArrays(1, &m_vao);
	m_vao = 0;
}

FrameHandle OpenGLRenderer::beginFrame()
{
	int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    glViewport(0, 0, width, height);
    glScissor(0, 0, width, height);
    glClearColor(0.5f, 0.5f, 0.5f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	return nullptr;
}

void OpenGLRenderer::endFrame()
{
	glfwSwapBuffers(m_window);
}

void OpenGLRenderer::waitIdle()
{

}

Vector2 OpenGLRenderer::getFrameBufferSize() const 
{
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);

    return Vector2(width, height);
}

void OpenGLRenderer::notifyFrameBufferResized(int _width, int _height)
{

}


void OpenGLRenderer::drawCommands(FrameHandle _frameHandle)
{
	YAE_CAPTURE_FUNCTION();

	glGetError();

	{
		YAE_CAPTURE_SCOPE("prepare buffers");

		YAE_GL_VERIFY(glBindVertexArray(m_vao));

		YAE_GL_VERIFY(glBindBuffer(GL_ARRAY_BUFFER, (GLuint)m_vertexBufferObject));
	    YAE_GL_VERIFY(glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(*m_vertices.data()), m_vertices.data(), GL_STATIC_DRAW));

	    YAE_GL_VERIFY(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)m_indexBufferObject));
	    YAE_GL_VERIFY(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(*m_indices.data()), m_indices.data(), GL_STATIC_DRAW));

	    YAE_GL_VERIFY(glEnableVertexAttribArray(0));
		YAE_GL_VERIFY(glEnableVertexAttribArray(1));
		YAE_GL_VERIFY(glEnableVertexAttribArray(2));
		YAE_GL_VERIFY(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)0));
		YAE_GL_VERIFY(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(sizeof(float)*3)));
		YAE_GL_VERIFY(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(sizeof(float)*6)));	
	}
	

	YAE_GL_VERIFY(glActiveTexture(GL_TEXTURE0));
    YAE_GL_VERIFY(glEnable(GL_DEPTH_TEST));
    YAE_GL_VERIFY(glDepthFunc(GL_LEQUAL));
    YAE_GL_VERIFY(glDisable(GL_STENCIL_TEST));
    YAE_GL_VERIFY(glEnable(GL_SCISSOR_TEST));
    YAE_GL_VERIFY(glEnable(GL_CULL_FACE));
    YAE_GL_VERIFY(glCullFace(GL_FRONT));
    YAE_GL_VERIFY(glEnable(GL_BLEND));
	YAE_GL_VERIFY(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    Matrix4 viewProj = m_projMatrix * m_viewMatrix;

	for (auto& pair : m_drawCommands)
	{
		YAE_CAPTURE_SCOPE("draw command pass");

		GLuint shader = (GLuint)pair.key;
		YAE_GL_VERIFY(glUseProgram(shader));

		GLint viewProjLocation = glGetUniformLocation((GLuint)shader, "viewProj");
		YAE_GL_VERIFY();
		if (viewProjLocation >= 0)
		{
			YAE_GL_VERIFY(glUniformMatrix4fv(viewProjLocation, 1, GL_FALSE, (float*)&viewProj));
		}

		GLint modelLocation = glGetUniformLocation((GLuint)shader, "model");
		YAE_GL_VERIFY();

		GLint textureLocation = glGetUniformLocation((GLuint)shader, "texture");
		YAE_GL_VERIFY();
		if (textureLocation >= 0)
		{
			YAE_GL_VERIFY(glUniform1i(textureLocation, 0));

		}

		for (DrawCommand& cmd : pair.value)
		{
			YAE_GL_VERIFY(glBindTexture(GL_TEXTURE_2D, (GLuint)cmd.textureId));

			if (modelLocation >= 0)
			{
				YAE_GL_VERIFY(glUniformMatrix4fv(modelLocation, 1, GL_FALSE, (float*)&cmd.transform));
			}

			void* offset = (void*)(intptr_t)(cmd.indexOffset * sizeof(*m_indices.data()));
    		YAE_GL_VERIFY(glDrawElements(
    			GL_TRIANGLES, 
    			cmd.elementCount, 
    			GL_UNSIGNED_INT, 
    			offset
    		));
		}

		pair.value.clear();
	}

	m_vertices.clear();
	m_indices.clear();
}

bool OpenGLRenderer::createTexture(void* _data, int _width, int _height, int _channels, TextureHandle& _outTextureHandle)
{
	YAE_CAPTURE_FUNCTION();

	GLuint internalFormat;
	GLuint format;

#if YAE_OPENGL_ES
	switch(_channels)
	{
		case 1:
		{
			internalFormat = GL_ALPHA;
			format = GL_ALPHA;
		}
		break;
		default:
		{
			internalFormat = GL_RGBA;
			format = GL_RGBA;
		}
		break;
	}
#else
	switch(_channels)
	{
		case 1:
		{
			internalFormat = GL_R8;
			format = GL_RED;
		}
		break;
		default:
		{
			internalFormat = GL_RGBA8;
			format = GL_RGBA;
		}
		break;
	}
#endif

	GLuint textureId;
    YAE_GL_VERIFY(glGenTextures(1, &textureId));
    YAE_GL_VERIFY(glBindTexture(GL_TEXTURE_2D, textureId));
    YAE_GL_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    YAE_GL_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    YAE_GL_VERIFY(glTexImage2D(
    	GL_TEXTURE_2D,    // target
    	0,                // level
    	internalFormat,   // internal format
    	_width,           // width
    	_height,          // height
    	0,                // border, must be 0
    	format,           // format
    	GL_UNSIGNED_BYTE, // type
    	_data
    ));

	_outTextureHandle = reinterpret_cast<void*>(textureId);
	return true;
}

void OpenGLRenderer::destroyTexture(TextureHandle& _inTextureHandle)
{
	YAE_CAPTURE_FUNCTION();

	GLuint textureId = reinterpret_cast<GLuint>(_inTextureHandle);
    YAE_GL_VERIFY(glDeleteTextures(1, &textureId));
}

bool OpenGLRenderer::createShader(ShaderType _type, const char* _code, size_t _codeSize, ShaderHandle& _outShaderHandle)
{
	YAE_CAPTURE_FUNCTION();

	GLenum glShaderType = 0;
	switch (_type)
	{
		case ShaderType::VERTEX: glShaderType = GL_VERTEX_SHADER; break;
		case ShaderType::GEOMETRY: glShaderType = GL_GEOMETRY_SHADER; break;
		case ShaderType::FRAGMENT: glShaderType = GL_FRAGMENT_SHADER; break;
		default: break;
	}

	const char* version = getShaderVersion();
	const char* defines = "";
#if YAE_OPENGL_ES
	defines = "#define OPENGL_ES\n";
#endif

	const char* code[] = {version, "\n", defines, _code};
	GLint codeSize[] = {-1, -1, -1, (GLint)_codeSize};
	GLuint shaderId = glCreateShader(glShaderType);
	YAE_ASSERT(shaderId != 0);
	YAE_GL_VERIFY(glShaderSource(shaderId, countof(code), code, codeSize));
	YAE_GL_VERIFY(glCompileShader(shaderId));

	GLint status = 0, logLength = 0;
    YAE_GL_VERIFY(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status));
    YAE_GL_VERIFY(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength));
    if ((GLboolean)status == GL_TRUE)
    {
    	_outShaderHandle = (void*)shaderId;
    }
    else
    {
    	YAE_ERROR("Failed to compile shader.");
    }

    if (logLength > 1)
    {
    	String buf(&scratchAllocator());
    	buf.resize(logLength);
        YAE_GL_VERIFY(glGetShaderInfoLog(shaderId, logLength, NULL, (GLchar*)buf.data()));
    	YAE_ERRORF("Shader compilation result:\n%s", buf.c_str());
    }

	return (GLboolean)status == GL_TRUE;
}

void OpenGLRenderer::destroyShader(ShaderHandle& _shaderHandle)
{
	YAE_CAPTURE_FUNCTION();

	GLuint shaderId = (GLuint)_shaderHandle;
	YAE_GL_VERIFY(glDeleteShader(shaderId));
}


bool OpenGLRenderer::createShaderProgram(ShaderHandle* _shaderHandles, u16 _shaderHandleCount, ShaderProgramHandle& _outShaderProgramHandle)
{
	YAE_CAPTURE_FUNCTION();

	GLuint programId = glCreateProgram();
	for (u16 i = 0; i < _shaderHandleCount; ++i)
	{
		YAE_ASSERT(_shaderHandles[i] != 0);
		GLint compiled;
        YAE_GL_VERIFY(glGetShaderiv((GLuint)_shaderHandles[i], GL_COMPILE_STATUS, &compiled));
        YAE_ASSERT(compiled != 0);
    	YAE_GL_VERIFY(glAttachShader(programId, (GLuint)_shaderHandles[i]));
	}
	YAE_GL_VERIFY(glLinkProgram(programId));

	GLint status = 0, logLength = 0;
    YAE_GL_VERIFY(glGetProgramiv(programId, GL_LINK_STATUS, &status));
    YAE_GL_VERIFY(glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLength));
    if ((GLboolean)status == GL_TRUE)
    {
    	_outShaderProgramHandle = (void*)programId;
    }
    else
    {
    	YAE_ERROR("Failed to link shader program.");
    }

    if (logLength > 1)
    {
    	String buf(&scratchAllocator());
    	buf.resize(logLength);
        YAE_GL_VERIFY(glGetProgramInfoLog(programId, logLength, NULL, (GLchar*)buf.data()));
    	YAE_ERRORF("Shader program linking result:\n%s", buf.c_str());
    }

    return (GLboolean)status == GL_TRUE;
}


void OpenGLRenderer::destroyShaderProgram(ShaderProgramHandle& _shaderProgramHandle)
{
	YAE_CAPTURE_FUNCTION();

	GLuint programId = (GLuint)_shaderProgramHandle;
	YAE_GL_VERIFY(glDeleteProgram(programId));
}


void OpenGLRenderer::drawMesh(const Matrix4& _transform, const Vertex* _vertices, u32 _verticesCount, const u32* _indices, u32 _indicesCount, const TextureHandle& _textureHandle)
{
	u32 shaderId = (u32)m_shader;
	DataArray<DrawCommand>* commandArray = m_drawCommands.get(shaderId);
	if (commandArray == nullptr)
	{
		commandArray = &m_drawCommands.set(shaderId, DataArray<DrawCommand>());
	}

	u32 baseIndex = m_vertices.size();
	u32 startIndex = m_indices.size();

	DrawCommand command;
	command.transform = _transform;
	command.indexOffset = startIndex;
	command.elementCount = _indicesCount;
	command.textureId = (u32)_textureHandle;
	commandArray->push_back(command);

	m_vertices.push_back(_vertices, _verticesCount);

	m_indices.resize(m_indices.size() + _indicesCount);
	for (u32 i = 0; i < _indicesCount; ++i)
	{
		m_indices[startIndex + i] = baseIndex + _indices[i];
	}
}


void OpenGLRenderer::drawText(const Matrix4& _transform, const FontResource* _font, const char* _text)
{
	glm::vec3 _color(1.f, 1.f, 1.f);

	u32 shaderId = (u32)m_fontShader;
	DataArray<DrawCommand>* commandArray = m_drawCommands.get(shaderId);
	if (commandArray == nullptr)
	{
		commandArray = &m_drawCommands.set(shaderId, DataArray<DrawCommand>());
	}

	u32 indicesStart = m_indices.size();
	u32 verticesStart = m_vertices.size();
	u32 textLength = strlen(_text);

	m_vertices.resize(m_vertices.size() + textLength * 4);
	m_indices.resize(m_indices.size() + textLength * 6);

	float xPos = 0.f;
	float yPos = 0.f;
	Vertex vertices[4];
	vertices[0].color = _color;
	vertices[1].color = _color;
	vertices[2].color = _color;
	vertices[3].color = _color;
	u32 indices[6] =
	{
		0, 1, 2,
		0, 2, 3
	};

	stbtt_aligned_quad quad;
	u32 indicesOffset = verticesStart;
	for (u32 i = 0; i < textLength; ++i)
	{

		stbtt_GetPackedQuad(
			_font->m_packedChar,
			_font->m_atlasWidth, _font->m_atlasHeight,
			_text[i],
			&xPos, &yPos,
			&quad,
			1
		);

		vertices[0].pos = glm::vec3(quad.x0, quad.y0, 0.f);
		vertices[1].pos = glm::vec3(quad.x1, quad.y0, 0.f);
		vertices[2].pos = glm::vec3(quad.x1, quad.y1, 0.f);
		vertices[3].pos = glm::vec3(quad.x0, quad.y1, 0.f);
		vertices[0].texCoord = glm::vec2(quad.s0, quad.t0);
		vertices[1].texCoord = glm::vec2(quad.s1, quad.t0);
		vertices[2].texCoord = glm::vec2(quad.s1, quad.t1);
		vertices[3].texCoord = glm::vec2(quad.s0, quad.t1);
		memcpy(m_vertices.data() + verticesStart + (i * 4), vertices, sizeof(*vertices) * 4);

		for (u32 j = 0; j < 6; ++j)
		{
			m_indices[indicesStart + (i * 6) + j] = indices[j] + indicesOffset;
		}
		indicesOffset += 4;
	}

	DrawCommand command;
	command.transform = _transform;
	command.indexOffset = indicesStart;
	command.elementCount = textLength * 6;
	command.textureId = (u32)_font->m_fontTexture;
	commandArray->push_back(command);
}

const char* OpenGLRenderer::getShaderVersion() const
{
	return OPENGL_SHADER_VERSION;
}

void OpenGLRenderer::initIm3d()
{
	YAE_CAPTURE_FUNCTION();

	FileResource* im3dShaderFile = findOrCreateResource<FileResource>("./data/shaders/im3d.glsl");
	im3dShaderFile->useLoad();
	YAE_ASSERT(im3dShaderFile->isLoaded());

	String shaderSource(&scratchAllocator());
	shaderSource.resize(im3dShaderFile->getContentSize());
	memcpy(shaderSource.data(), im3dShaderFile->getContent(), im3dShaderFile->getContentSize());

	// Points
	{
		String vsSource = String("#define VERTEX_SHADER\n#define POINTS\n", &scratchAllocator()) + shaderSource;
		String fsSource = String("#define FRAGMENT_SHADER\n#define POINTS\n", &scratchAllocator()) + shaderSource;

		ShaderHandle vs;
		YAE_VERIFY(createShader(ShaderType::VERTEX, vsSource.c_str(), vsSource.size(), vs));

		ShaderHandle fs;
		YAE_VERIFY(createShader(ShaderType::FRAGMENT, fsSource.c_str(), fsSource.size(), fs));

		ShaderHandle shaders[] = {vs, fs};
		YAE_VERIFY(createShaderProgram(shaders, countof(shaders), m_im3dShaderPoints));

		destroyShader(fs);
		destroyShader(vs);
	}

	// Lines
	{
		String vsSource = String("#define VERTEX_SHADER\n#define LINES\n", &scratchAllocator()) + shaderSource;
		String gsSource = String("#define GEOMETRY_SHADER\n#define LINES\n", &scratchAllocator()) + shaderSource;
		String fsSource = String("#define FRAGMENT_SHADER\n#define LINES\n", &scratchAllocator()) + shaderSource;

		ShaderHandle vs;
		YAE_VERIFY(createShader(ShaderType::VERTEX, vsSource.c_str(), vsSource.size(), vs));

		ShaderHandle gs;
		YAE_VERIFY(createShader(ShaderType::GEOMETRY, gsSource.c_str(), gsSource.size(), gs));

		ShaderHandle fs;
		YAE_VERIFY(createShader(ShaderType::FRAGMENT, fsSource.c_str(), fsSource.size(), fs));

		ShaderHandle shaders[] = {vs, gs, fs};
		YAE_VERIFY(createShaderProgram(shaders, countof(shaders), m_im3dShaderLines));

		destroyShader(fs);
		destroyShader(gs);
		destroyShader(vs);
	}

	// Triangles
	{
		String vsSource = String("#define VERTEX_SHADER\n#define TRIANGLES\n", &scratchAllocator()) + shaderSource;
		String fsSource = String("#define FRAGMENT_SHADER\n#define TRIANGLES\n", &scratchAllocator()) + shaderSource;

		ShaderHandle vs;
		YAE_VERIFY(createShader(ShaderType::VERTEX, vsSource.c_str(), vsSource.size(), vs));

		ShaderHandle fs;
		YAE_VERIFY(createShader(ShaderType::FRAGMENT, fsSource.c_str(), fsSource.size(), fs));

		ShaderHandle shaders[] = {vs, fs};
		YAE_VERIFY(createShaderProgram(shaders, countof(shaders), m_im3dShaderTriangles));

		destroyShader(fs);
		destroyShader(vs);
	}
	im3dShaderFile->releaseUnuse();

	YAE_GL_VERIFY(glGenBuffers(1, &m_im3dVertexBuffer));;
	YAE_GL_VERIFY(glGenVertexArrays(1, &m_im3dVertexArray));
	YAE_GL_VERIFY(glBindVertexArray(m_im3dVertexArray));
	YAE_GL_VERIFY(glBindBuffer(GL_ARRAY_BUFFER, m_im3dVertexBuffer));
	YAE_GL_VERIFY(glEnableVertexAttribArray(0));
	YAE_GL_VERIFY(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Im3d::VertexData), (GLvoid*)offsetof(Im3d::VertexData, m_positionSize)));
	YAE_GL_VERIFY(glEnableVertexAttribArray(1));
	YAE_GL_VERIFY(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Im3d::VertexData), (GLvoid*)offsetof(Im3d::VertexData, m_color)));
	YAE_GL_VERIFY(glBindVertexArray(0));
}

void OpenGLRenderer::shutdownIm3d()
{
	YAE_CAPTURE_FUNCTION();

	YAE_GL_VERIFY(glDeleteVertexArrays(1, &m_im3dVertexArray));
	YAE_GL_VERIFY(glDeleteBuffers(1, &m_im3dVertexBuffer));

	destroyShaderProgram(m_im3dShaderPoints);
	m_im3dShaderPoints = nullptr;

	destroyShaderProgram(m_im3dShaderLines);
	m_im3dShaderPoints = nullptr;

	destroyShaderProgram(m_im3dShaderTriangles);
	m_im3dShaderPoints = nullptr;
}

void OpenGLRenderer::drawIm3d(const Im3d::DrawList* _drawLists, u32 _drawListCount)
{
	YAE_CAPTURE_FUNCTION();

	const Vector2 viewportSize = getFrameBufferSize();
	const Matrix4 viewProj = m_projMatrix * m_viewMatrix;

	YAE_GL_VERIFY(glViewport(0, 0, (GLsizei)viewportSize.x, (GLsizei)viewportSize.y));
	YAE_GL_VERIFY(glEnable(GL_BLEND));
	YAE_GL_VERIFY(glBlendEquation(GL_FUNC_ADD));
	YAE_GL_VERIFY(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	YAE_GL_VERIFY(glEnable(GL_PROGRAM_POINT_SIZE));
		
	for (u32 i = 0; i < _drawListCount; ++i)
	{
		const Im3d::DrawList& drawList = _drawLists[i];
 
		if (drawList.m_layerId == Im3d::MakeId("NamedLayer"))
		{
		 // The application may group primitives into layers, which can be used to change the draw state (e.g. enable depth testing, use a different shader)
		}
	
		GLenum prim;
		GLuint sh;
		switch (drawList.m_primType)
		{
			case Im3d::DrawPrimitive_Points:
				prim = GL_POINTS;
				sh = (GLuint)m_im3dShaderPoints;
				YAE_GL_VERIFY(glDisable(GL_CULL_FACE)); // points are view-aligned
				break;
			case Im3d::DrawPrimitive_Lines:
				prim = GL_LINES;
				sh = (GLuint)m_im3dShaderLines;
				YAE_GL_VERIFY(glDisable(GL_CULL_FACE)); // lines are view-aligned
				break;
			case Im3d::DrawPrimitive_Triangles:
				prim = GL_TRIANGLES;
				sh = (GLuint)m_im3dShaderTriangles;
				//YAE_GL_VERIFY(glEnable(GL_CULL_FACE)); // culling valid for triangles, but optional
				break;
			default:
				YAE_ASSERT(false);
				return;
		};
	
		YAE_GL_VERIFY(glBindVertexArray(m_im3dVertexArray));
		YAE_GL_VERIFY(glBindBuffer(GL_ARRAY_BUFFER, m_im3dVertexBuffer));
		YAE_GL_VERIFY(glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)drawList.m_vertexCount * sizeof(Im3d::VertexData), (GLvoid*)drawList.m_vertexData, GL_STREAM_DRAW));
	
		YAE_GL_VERIFY(glUseProgram(sh));
		YAE_GL_VERIFY(glUniform2f(glGetUniformLocation(sh, "uViewport"), viewportSize.x, viewportSize.y));
		YAE_GL_VERIFY(glUniformMatrix4fv(glGetUniformLocation(sh, "uViewProjMatrix"), 1, false, (const GLfloat*)&viewProj));
		YAE_GL_VERIFY(glDrawArrays(prim, 0, (GLsizei)drawList.m_vertexCount));
	}

	// Text rendering.
 	// This is common to all examples since we're using ImGui to draw the text lists, see im3d_example.cpp.
	//g_Example->drawTextDrawListsImGui(Im3d::GetTextDrawLists(), Im3d::GetTextDrawListCount());
}

} // namespace yae
