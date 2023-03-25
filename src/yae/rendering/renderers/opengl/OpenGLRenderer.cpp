#include "OpenGLRenderer.h"

#include <yae/program.h>
#include <yae/resources/ShaderFile.h>
#include <yae/resources/FontFile.h>
#include <yae/resources/File.h>
#include <yae/resource.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#else
#include <GL/gl3w.h>
#endif
#include <imgui/backends/imgui_impl_opengl3.h>
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

#define YAE_GL_VERIFY(_instruction) { _instruction; GLint ___error = glGetError(); YAE_ASSERT_MSGF(___error == GL_NO_ERROR, "GL Error %s(0x%04x) -> " #_instruction, glErrorToString(___error), ___error); }

namespace yae {

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

bool OpenGLRenderer::_init()
{	
	glfwMakeContextCurrent(m_window);
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

	YAE_GL_VERIFY(glBindVertexArray(0));

	// Quad
	{
		YAE_GL_VERIFY(glGenVertexArrays(1, &m_quadVertexArray));
		YAE_GL_VERIFY(glBindVertexArray(m_quadVertexArray));
		static const GLfloat g_quad_vertex_buffer_data[] = {
			// vertex 				// uv
			-1.0f, -1.0f, 0.0f,		0.0f, 0.0f,
			1.0f, -1.0f, 0.0f,		1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,		0.0f, 1.0f,
			-1.0f,  1.0f, 0.0f,		0.0f, 1.0f,
			1.0f, -1.0f, 0.0f,		1.0f, 0.0f,
			1.0f,  1.0f, 0.0f,		1.0f, 1.0f,
		};

		YAE_GL_VERIFY(glGenBuffers(1, &m_quadVertexBuffer));
		YAE_GL_VERIFY(glBindBuffer(GL_ARRAY_BUFFER, m_quadVertexBuffer));
		YAE_GL_VERIFY(glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW));

		YAE_GL_VERIFY(glEnableVertexAttribArray(0));
		YAE_GL_VERIFY(glEnableVertexAttribArray(1));
		YAE_GL_VERIFY(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, (const GLvoid*)0));
		YAE_GL_VERIFY(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, (const GLvoid*)(sizeof(float)*3)));

		YAE_GL_VERIFY(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	return true;
}

void OpenGLRenderer::_shutdown()
{
	glDeleteBuffers(1, &m_quadVertexBuffer);
	m_quadVertexBuffer = 0;
	glDeleteVertexArrays(1, &m_quadVertexArray);
	m_quadVertexArray = 0;

	GLuint buffers[2] = { m_vertexBufferObject, m_indexBufferObject};
	glDeleteBuffers(2, buffers);
	m_vertexBufferObject = 0;
	m_indexBufferObject = 0;

	glDeleteVertexArrays(1, &m_vao);
	m_vao = 0;
}

void OpenGLRenderer::waitIdle()
{

}

bool OpenGLRenderer::createTexture(const void* _data, int _width, int _height, int _channels, TextureHandle& _outTextureHandle)
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

	_outTextureHandle = textureId;
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
    	_outShaderHandle = shaderId;
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
    	_outShaderProgramHandle = programId;
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

    YAE_GL_VERIFY(glBindAttribLocation(programId, 0, "inPosition"));
	YAE_GL_VERIFY(glBindAttribLocation(programId, 1, "inColor"));
	YAE_GL_VERIFY(glBindAttribLocation(programId, 2, "inTexCoord"));

    return (GLboolean)status == GL_TRUE;
}


void OpenGLRenderer::destroyShaderProgram(ShaderProgramHandle& _shaderProgramHandle)
{
	YAE_CAPTURE_FUNCTION();

	GLuint programId = (GLuint)_shaderProgramHandle;
	YAE_GL_VERIFY(glDeleteProgram(programId));
}

const char* OpenGLRenderer::getShaderVersion() const
{
	return OPENGL_SHADER_VERSION;
}

bool OpenGLRenderer::_initImGui()
{
	YAE_CAPTURE_FUNCTION();

	return ImGui_ImplOpenGL3_Init(getShaderVersion());
}

void OpenGLRenderer::_renderImGui()
{
	YAE_GL_VERIFY(glBindFramebuffer(GL_FRAMEBUFFER, 0)); 

	ImDrawData* imguiDrawData = ImGui::GetDrawData();
	const bool isMinimized = (imguiDrawData->DisplaySize.x <= 0.0f || imguiDrawData->DisplaySize.y <= 0.0f);
	if (!isMinimized)
	{
		ImGui_ImplOpenGL3_RenderDrawData(imguiDrawData);
	}
}

void OpenGLRenderer::_shutdownImGui()
{
	YAE_CAPTURE_FUNCTION();

	ImGui_ImplOpenGL3_Shutdown();
}

void OpenGLRenderer::_initRenderTarget(RenderTarget& _renderTarget)
{
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(_renderTarget.m_width != 0);
	YAE_ASSERT(_renderTarget.m_height != 0);

	GLuint textures[2];
	YAE_GL_VERIFY(glGenTextures(2, textures));
	_renderTarget.m_renderTexture = (TextureHandle)textures[0];
	_renderTarget.m_depthTexture = (TextureHandle)textures[1];

	// Render Texture
	YAE_GL_VERIFY(glBindTexture(GL_TEXTURE_2D, (GLuint)_renderTarget.m_renderTexture));

	YAE_GL_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	YAE_GL_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	YAE_GL_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
	YAE_GL_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
	YAE_GL_VERIFY(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _renderTarget.m_width, _renderTarget.m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));

	YAE_GL_VERIFY(glBindTexture(GL_TEXTURE_2D, 0));

	// Depth Texture
	YAE_GL_VERIFY(glBindTexture(GL_TEXTURE_2D, (GLuint)_renderTarget.m_depthTexture));

	YAE_GL_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	YAE_GL_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	YAE_GL_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	YAE_GL_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	YAE_GL_VERIFY(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _renderTarget.m_width, _renderTarget.m_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL));

	YAE_GL_VERIFY(glBindTexture(GL_TEXTURE_2D, 0));

	// Frame Buffer
	YAE_GL_VERIFY(glGenFramebuffers(1, (GLuint*)&_renderTarget.m_frameBuffer));
	YAE_GL_VERIFY(glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)_renderTarget.m_frameBuffer));

	YAE_GL_VERIFY(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, (GLuint)_renderTarget.m_renderTexture, 0));
	YAE_GL_VERIFY(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, (GLuint)_renderTarget.m_depthTexture, 0));
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		// What should we do if it fails ? let's think about this when it happens
		YAE_ASSERT(false);
	}

	YAE_GL_VERIFY(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void OpenGLRenderer::_resizeRenderTarget(RenderTarget& _renderTarget)
{
	YAE_ASSERT(_renderTarget.m_width != 0);
	YAE_ASSERT(_renderTarget.m_height != 0);

	YAE_GL_VERIFY(glBindTexture(GL_TEXTURE_2D, (GLuint)_renderTarget.m_renderTexture));
	YAE_GL_VERIFY(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _renderTarget.m_width, _renderTarget.m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));

	YAE_GL_VERIFY(glBindTexture(GL_TEXTURE_2D, (GLuint)_renderTarget.m_depthTexture));
	YAE_GL_VERIFY(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _renderTarget.m_width, _renderTarget.m_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL));

	YAE_GL_VERIFY(glBindTexture(GL_TEXTURE_2D, 0));
}

void OpenGLRenderer::_shutdownRenderTarget(RenderTarget& _renderTarget)
{
	YAE_CAPTURE_FUNCTION();

	GLuint textures[2] = {(GLuint)_renderTarget.m_renderTexture, (GLuint)_renderTarget.m_depthTexture };
    YAE_GL_VERIFY(glDeleteTextures(2, textures));
    YAE_GL_VERIFY(glDeleteFramebuffers(1, (GLuint*)&_renderTarget.m_frameBuffer));
}

void OpenGLRenderer::_beginFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
}

void OpenGLRenderer::_beginRender()
{
	YAE_GL_VERIFY(glBindFramebuffer(GL_FRAMEBUFFER, 0)); 

	Vector2 frameBufferSize = getFrameBufferSize();
    glScissor(0, 0, frameBufferSize.x, frameBufferSize.y);
}

void OpenGLRenderer::_renderCamera(const RenderCamera* _camera)
{
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(_camera != nullptr);
	YAE_ASSERT(_camera->m_scene != nullptr);

	glGetError();

	{
		YAE_CAPTURE_SCOPE("prepare state");

		YAE_GL_VERIFY(glBindVertexArray(m_vao));

		YAE_GL_VERIFY(glBindBuffer(GL_ARRAY_BUFFER, (GLuint)m_vertexBufferObject));
	    YAE_GL_VERIFY(glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(*m_vertices.data()), m_vertices.data(), GL_STATIC_DRAW));

	    YAE_GL_VERIFY(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)m_indexBufferObject));
	    YAE_GL_VERIFY(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(*m_indices.data()), m_indices.data(), GL_STATIC_DRAW));

	    YAE_GL_VERIFY(glActiveTexture(GL_TEXTURE0));
	    YAE_GL_VERIFY(glEnable(GL_DEPTH_TEST));
	    YAE_GL_VERIFY(glDepthFunc(GL_LEQUAL));
	    YAE_GL_VERIFY(glDisable(GL_STENCIL_TEST));
	    YAE_GL_VERIFY(glEnable(GL_SCISSOR_TEST));
	    YAE_GL_VERIFY(glEnable(GL_CULL_FACE));
	    YAE_GL_VERIFY(glFrontFace(GL_CW));
	    YAE_GL_VERIFY(glCullFace(GL_BACK));
	    YAE_GL_VERIFY(glEnable(GL_BLEND));
		YAE_GL_VERIFY(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	}

	Vector2 viewportSize = _camera->getViewportSize();
    Matrix4 viewProj = _camera->computeViewProjectionMatrix();
    RenderScene* scene = _camera->m_scene;

	{
		YAE_CAPTURE_SCOPE("clear");

		if (_camera->renderTarget != nullptr)
		{
			YAE_GL_VERIFY(glBindFramebuffer(GL_FRAMEBUFFER, _camera->renderTarget->m_frameBuffer));
		}
		else
		{
			YAE_GL_VERIFY(glBindFramebuffer(GL_FRAMEBUFFER, 0)); 

		}
		YAE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	    glViewport(0, 0, viewportSize.x, viewportSize.y);
	    glScissor(0, 0, viewportSize.x, viewportSize.y);
	    glClearColor(_camera->clearColor.x, _camera->clearColor.y, _camera->clearColor.z, _camera->clearColor.w);
	    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	}

	for (auto& pair : scene->m_drawCommands)
	{
		YAE_CAPTURE_SCOPE("draw command pass");

		GLuint shader = (GLuint)pair.key;
		if (shader == 0)
			continue;

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

	YAE_GL_VERIFY(glBindBuffer(GL_ARRAY_BUFFER, 0));
    YAE_GL_VERIFY(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    YAE_GL_VERIFY(glBindVertexArray(0));
}

void OpenGLRenderer::_endRender()
{
	glfwSwapBuffers(m_window);	
}

void OpenGLRenderer::_endFrame()
{
}

bool OpenGLRenderer::_initIm3d()
{
	YAE_CAPTURE_FUNCTION();

	File* im3dShaderFile = resource::findOrCreateFile<File>("./data/shaders/im3d.glsl");
	im3dShaderFile->load();
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
	im3dShaderFile->release();

	YAE_GL_VERIFY(glGenBuffers(1, &m_im3dVertexBuffer));;
	YAE_GL_VERIFY(glGenVertexArrays(1, &m_im3dVertexArray));
	YAE_GL_VERIFY(glBindVertexArray(m_im3dVertexArray));
	YAE_GL_VERIFY(glBindBuffer(GL_ARRAY_BUFFER, m_im3dVertexBuffer));
	YAE_GL_VERIFY(glEnableVertexAttribArray(0));
	YAE_GL_VERIFY(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Im3d::VertexData), (GLvoid*)offsetof(Im3d::VertexData, m_positionSize)));
	YAE_GL_VERIFY(glEnableVertexAttribArray(1));
	YAE_GL_VERIFY(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Im3d::VertexData), (GLvoid*)offsetof(Im3d::VertexData, m_color)));
	YAE_GL_VERIFY(glBindVertexArray(0));

	return true;
}

void OpenGLRenderer::_shutdownIm3d()
{
	YAE_CAPTURE_FUNCTION();

	YAE_GL_VERIFY(glDeleteVertexArrays(1, &m_im3dVertexArray));
	YAE_GL_VERIFY(glDeleteBuffers(1, &m_im3dVertexBuffer));

	destroyShaderProgram(m_im3dShaderPoints);
	m_im3dShaderPoints = 0;

	destroyShaderProgram(m_im3dShaderLines);
	m_im3dShaderPoints = 0;

	destroyShaderProgram(m_im3dShaderTriangles);
	m_im3dShaderPoints = 0;
}

void OpenGLRenderer::_renderIm3d(const RenderCamera* _camera)
{
	YAE_CAPTURE_FUNCTION();

	const Vector2 viewportSize = getFrameBufferSize();
	const Matrix4 viewProj = _camera->computeViewProjectionMatrix();

    YAE_GL_VERIFY(glEnable(GL_DEPTH_TEST));
    YAE_GL_VERIFY(glDepthFunc(GL_LEQUAL));
	YAE_GL_VERIFY(glEnable(GL_BLEND));
	YAE_GL_VERIFY(glBlendEquation(GL_FUNC_ADD));
	YAE_GL_VERIFY(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	YAE_GL_VERIFY(glEnable(GL_PROGRAM_POINT_SIZE));
		
	for (u32 i = 0; i < Im3d::GetDrawListCount(); ++i)
	{
		const Im3d::DrawList& drawList = Im3d::GetDrawLists()[i];
 
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
}

} // namespace yae
