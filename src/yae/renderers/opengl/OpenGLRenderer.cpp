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
	switch(_severity)
	{
		case GL_DEBUG_SEVERITY_HIGH: YAE_ERRORF_CAT("OpenGL", "gldebug 0x%04x:0x%04x: %s", _source, _type, _msg); break;
		case GL_DEBUG_SEVERITY_MEDIUM: YAE_WARNINGF_CAT("OpenGL", "gldebug 0x%04x:0x%04x: %s", _source, _type, _msg); break;
		default: YAE_VERBOSEF_CAT("OpenGL", "gldebug 0x%04x:0x%04x: %s", _source, _type, _msg); break;
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

	m_texture = findOrCreateResource<TextureResource>("./data/textures/viking_room.png");
	m_texture->useLoad();
	YAE_ASSERT(m_texture->isLoaded());

	m_vertexShader = findOrCreateResource<ShaderResource>("./data/shaders/shader.vert", ShaderType::VERTEX, "main");
	m_vertexShader->useLoad();
	YAE_ASSERT(m_vertexShader->isLoaded());

	m_fragmentShader = findOrCreateResource<ShaderResource>("./data/shaders/shader.frag", ShaderType::FRAGMENT, "main");
	m_fragmentShader->useLoad();
	YAE_ASSERT(m_fragmentShader->isLoaded());

	ShaderHandle shaders[] =
	{
		m_vertexShader->getShaderHandle(),
		m_fragmentShader->getShaderHandle()
	};
	YAE_VERIFY(createShaderProgram(shaders, countof(shaders), m_shader));

	m_uniformLocation_view = glGetUniformLocation((GLuint)m_shader, "view");
	YAE_GL_VERIFY();
	YAE_ASSERT(m_uniformLocation_view >= 0);

	m_uniformLocation_proj = glGetUniformLocation((GLuint)m_shader, "proj");
	YAE_GL_VERIFY();
	YAE_ASSERT(m_uniformLocation_proj >= 0);

	m_uniformLocation_model = glGetUniformLocation((GLuint)m_shader, "model");
	YAE_GL_VERIFY();
	YAE_ASSERT(m_uniformLocation_model >= 0);

	m_uniformLocation_texSampler = glGetUniformLocation((GLuint)m_shader, "texSampler");
	YAE_GL_VERIFY();
	YAE_ASSERT(m_uniformLocation_texSampler >= 0);

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

	m_texture->releaseUnuse();
	m_texture = nullptr;
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

    YAE_GL_VERIFY(glUseProgram((GLuint)m_shader));

    YAE_GL_VERIFY(glBindTexture(GL_TEXTURE_2D, (GLuint)m_texture->getTextureHandle()));
	YAE_GL_VERIFY(glActiveTexture(GL_TEXTURE0));
    YAE_GL_VERIFY(glEnable(GL_DEPTH_TEST));
    YAE_GL_VERIFY(glDisable(GL_STENCIL_TEST));
    YAE_GL_VERIFY(glEnable(GL_SCISSOR_TEST));
    YAE_GL_VERIFY(glEnable(GL_CULL_FACE));
    YAE_GL_VERIFY(glCullFace(GL_FRONT));

    YAE_GL_VERIFY(glUniformMatrix4fv(m_uniformLocation_view, 1, GL_FALSE, (float*)&m_viewMatrix));
    YAE_GL_VERIFY(glUniformMatrix4fv(m_uniformLocation_proj, 1, GL_FALSE, (float*)&m_projMatrix));
	YAE_GL_VERIFY(glUniform1i(m_uniformLocation_texSampler, 0));

    for (DrawCommand cmd : m_drawCommands)
    {
    	GLMeshHandle* meshHandle = (GLMeshHandle*)cmd.mesh;

    	YAE_GL_VERIFY(glBindBuffer(GL_ARRAY_BUFFER, (GLuint)meshHandle->vertexBufferId));
    	YAE_GL_VERIFY(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)meshHandle->indexBufferId));

    	YAE_GL_VERIFY(glEnableVertexAttribArray(0));
    	YAE_GL_VERIFY(glEnableVertexAttribArray(1));
    	YAE_GL_VERIFY(glEnableVertexAttribArray(2));

    	YAE_GL_VERIFY(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)0));
    	YAE_GL_VERIFY(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(sizeof(float)*3)));
    	YAE_GL_VERIFY(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(sizeof(float)*6)));

    	YAE_GL_VERIFY(glUniformMatrix4fv(m_uniformLocation_model, 1, GL_FALSE, (float*)&cmd.transform));

        YAE_GL_VERIFY(glDrawElements(GL_TRIANGLES, meshHandle->indicesCount, GL_UNSIGNED_INT, 0));
    }
    m_drawCommands.clear();
}

bool OpenGLRenderer::createTexture(void* _data, int _width, int _height, int _channels, TextureHandle& _outTextureHandle)
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
    	GL_RGBA,          // internal format
    	_width,           // width
    	_height,          // height
    	0,                // border, must be 0
    	GL_RGBA,          // format
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

bool OpenGLRenderer::createMesh(Vertex* _vertices, u32 _verticesCount, u32* _indices, u32 _indicesCount, MeshHandle& _outMeshHandle)
{
	YAE_CAPTURE_FUNCTION();

	GLuint buffers[2]; // 0 is vertices, 1 is indices
    YAE_GL_VERIFY(glGenBuffers(2, buffers));

    YAE_GL_VERIFY(glBindBuffer(GL_ARRAY_BUFFER, buffers[0]));
    YAE_GL_VERIFY(glBufferData(GL_ARRAY_BUFFER, _verticesCount * sizeof(*_vertices), _vertices, GL_STATIC_DRAW));

    YAE_GL_VERIFY(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]));
    YAE_GL_VERIFY(glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indicesCount * sizeof(*_indices), _indices, GL_STATIC_DRAW));

    GLMeshHandle* meshHandle = defaultAllocator().create<GLMeshHandle>();
    meshHandle->vertexBufferId = buffers[0];
    meshHandle->indexBufferId = buffers[1];
    meshHandle->indicesCount = _indicesCount;
    _outMeshHandle = meshHandle;

	return true;
}

void OpenGLRenderer::destroyMesh(MeshHandle& _inMeshHandle)
{
	YAE_CAPTURE_FUNCTION();

	GLMeshHandle* meshHandle = reinterpret_cast<GLMeshHandle*>(_inMeshHandle);
    YAE_GL_VERIFY(glDeleteBuffers(1, &meshHandle->vertexBufferId));
    YAE_GL_VERIFY(glDeleteBuffers(1, &meshHandle->indexBufferId));
    defaultAllocator().destroy(meshHandle);
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


void OpenGLRenderer::drawMesh(const Matrix4& _transform, const MeshHandle& _meshHandle)
{
	DrawCommand command;
	command.transform = _transform;
	command.mesh = _meshHandle;
	m_drawCommands.push_back(command);
}

const char* OpenGLRenderer::getShaderVersion() const
{
	return OPENGL_SHADER_VERSION;
}


} // namespace yae
