#include "OpenGLRenderer.h"

#include <yae/resources/TextureResource.h>
#include <yae/resources/ShaderResource.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#else
#include <GL/gl3w.h>
#endif
#include <imgui/backends/imgui_impl_glfw.h>
#include <GLFW/glfw3.h>

const int OPENGL_VERSION_MAJOR = 2;
const int OPENGL_VERSION_MINOR = 0;
const char* OPENGL_SHADER_VERSION = "#version 100";

#define YAE_GL_VERIFY(_instruction) { _instruction; GLint ___error = glGetError(); YAE_ASSERT_MSGF(___error == GL_NO_ERROR, "GL Error 0x%04x -> " #_instruction, ___error); }

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
		case GL_DEBUG_SEVERITY_HIGH: YAE_ERRORF_CAT("OpenGL", "gldebug 0x%04x:0x%04x:0x%04x: %s", _source, _type, _id, _msg); break;
		case GL_DEBUG_SEVERITY_MEDIUM: YAE_WARNINGF_CAT("OpenGL", "gldebug 0x%04x:0x%04x:0x%04x: %s", _source, _type, _id, _msg); break;
		default: YAE_VERBOSEF_CAT("OpenGL", "gldebug 0x%04x:0x%04x:0x%04x: %s", _source, _type, _id, _msg); break;
	}
}

void OpenGLRenderer::hintWindow()
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
}

bool OpenGLRenderer::init(GLFWwindow* _window)
{
	glfwMakeContextCurrent(_window);
    glfwSwapInterval(1); // Enable vsync

#if YAE_PLATFORM_WEB == 0
	// TODO: Move gl3w init to the device part
	if (gl3wIsSupported(OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR) != GL3W_OK)
	{
		YAE_ERRORF_CAT("opengl", "Failed to initialize gl3w: Version %d.%d not supported", OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR);
		return false;
	}

	if (gl3wInit() != GL3W_OK)
	{
		YAE_ERROR_CAT("opengl", "Failed to initialize gl3w");
		return false;
	}

	GLint v;
	glGetIntegerv(GL_CONTEXT_FLAGS, &v);
	if (v & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		YAE_VERBOSE_CAT("OpenGL", "OpenGL debug context present");
		glDebugMessageCallback(&glDebugCallback, nullptr);
	}
#endif

	GLuint buffers[2] = {} ; // 0 is vertices, 1 is indices
    YAE_GL_VERIFY(glGenBuffers(2, buffers));
    m_vertexBufferObject = buffers[0];
    m_indexBufferObject = buffers[1];

	YAE_GL_VERIFY(glBindBuffer(GL_ARRAY_BUFFER, (GLuint)m_vertexBufferObject));
    YAE_GL_VERIFY(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)m_indexBufferObject));
	YAE_GL_VERIFY(glEnableVertexAttribArray(0));
	YAE_GL_VERIFY(glEnableVertexAttribArray(1));
	YAE_GL_VERIFY(glEnableVertexAttribArray(2));
	YAE_GL_VERIFY(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)0));
	YAE_GL_VERIFY(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(sizeof(float)*3)));
	YAE_GL_VERIFY(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(sizeof(float)*6)));

	m_vertexShader = findOrCreateResource<ShaderResource>("./data/shaders/shader.vert", ShaderType::VERTEX);
	m_vertexShader->useLoad();
	YAE_ASSERT(m_vertexShader->isLoaded());

	m_fragmentShader = findOrCreateResource<ShaderResource>("./data/shaders/shader.frag", ShaderType::FRAGMENT);
	m_fragmentShader->useLoad();
	YAE_ASSERT(m_fragmentShader->isLoaded());

	ShaderHandle shaders[] =
	{
		m_vertexShader->getShaderHandle(),
		m_fragmentShader->getShaderHandle()
	};
	YAE_VERIFY(createShaderProgram(shaders, countof(shaders), m_shader));

	YAE_GL_VERIFY(glBindAttribLocation((GLuint)m_shader, 0, "inPosition"));
	YAE_GL_VERIFY(glBindAttribLocation((GLuint)m_shader, 1, "inColor"));
	YAE_GL_VERIFY(glBindAttribLocation((GLuint)m_shader, 2, "inTexCoord"));

	m_window = _window;
	return true;
}

void OpenGLRenderer::shutdown()
{
	destroyShaderProgram(m_shader);
	m_shader = nullptr;

	m_fragmentShader->releaseUnuse();
	m_fragmentShader = nullptr;

	m_vertexShader->releaseUnuse();
	m_vertexShader = nullptr;
}

FrameHandle OpenGLRenderer::beginFrame()
{
	int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(0, 0, 0, 1);
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
	glGetError();

	YAE_GL_VERIFY(glBindBuffer(GL_ARRAY_BUFFER, (GLuint)m_vertexBufferObject));
    YAE_GL_VERIFY(glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(*m_vertices.data()), m_vertices.data(), GL_STATIC_DRAW));

    YAE_GL_VERIFY(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)m_indexBufferObject));
    YAE_GL_VERIFY(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(*m_indices.data()), m_indices.data(), GL_STATIC_DRAW));

	YAE_GL_VERIFY(glActiveTexture(GL_TEXTURE0));
    YAE_GL_VERIFY(glEnable(GL_DEPTH_TEST));
    YAE_GL_VERIFY(glDisable(GL_STENCIL_TEST));
    YAE_GL_VERIFY(glEnable(GL_SCISSOR_TEST));
    YAE_GL_VERIFY(glEnable(GL_CULL_FACE));
    YAE_GL_VERIFY(glCullFace(GL_FRONT));

    Matrix4 viewProj = m_projMatrix * m_viewMatrix;

	for (auto& pair : m_drawCommands)
	{
		YAE_GL_VERIFY(glUseProgram((GLuint)pair.key));

		GLint viewProjLocation = glGetUniformLocation((GLuint)m_shader, "viewProj");
		YAE_GL_VERIFY();
		if (viewProjLocation >= 0)
		{
			YAE_GL_VERIFY(glUniformMatrix4fv(viewProjLocation, 1, GL_FALSE, (float*)&viewProj));
		}

		GLint modelLocation = glGetUniformLocation((GLuint)m_shader, "model");
		YAE_GL_VERIFY();

		GLint textureLocation = glGetUniformLocation((GLuint)m_shader, "texture");
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

bool OpenGLRenderer::createTexture(void* _data, int _width, int _height, int _format, TextureHandle& _outTextureHandle)
{
	YAE_CAPTURE_FUNCTION();

	GLuint textureId;
    YAE_GL_VERIFY(glGenTextures(1, &textureId));
    YAE_GL_VERIFY(glBindTexture(GL_TEXTURE_2D, textureId));
    YAE_GL_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    YAE_GL_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    YAE_GL_VERIFY(glTexImage2D(
    	GL_TEXTURE_2D,    // target
    	0,                // level
    	_format,          // internal format
    	_width,           // width
    	_height,          // height
    	0,                // border, must be 0
    	_format,          // format
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
		case ShaderType::FRAGMENT: glShaderType = GL_FRAGMENT_SHADER; break;
		default: break;
	}

	GLint codeSize = _codeSize;
	GLuint shaderId = glCreateShader(glShaderType);
	YAE_ASSERT(shaderId != 0);
	YAE_GL_VERIFY(glShaderSource(shaderId, 1, &_code, &codeSize));
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

}


const char* OpenGLRenderer::getShaderVersion() const
{
	return OPENGL_SHADER_VERSION;
}


} // namespace yae
