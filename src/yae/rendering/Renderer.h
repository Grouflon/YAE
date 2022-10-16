#pragma once

#include <yae/types.h>
#include <yae/rendering/render_types.h>
#include <yae/math_types.h>

typedef struct GLFWwindow GLFWwindow;

namespace Im3d
{
	struct DrawList;	
}

namespace yae {

class FontResource;

class YAE_API Renderer
{
public:
	virtual ~Renderer() {}

	virtual bool init(GLFWwindow* _window) = 0;
	virtual void waitIdle() = 0;
	virtual void shutdown() = 0;

	virtual RendererType getType() const = 0;

	virtual void hintWindow() {}

	virtual FrameHandle beginFrame() = 0;
  	virtual void endFrame() = 0;

	virtual Vector2 getFrameBufferSize() const { return Vector2::ZERO; }
	virtual void notifyFrameBufferResized(int _width, int _height) {}

	virtual void drawCommands(FrameHandle _frameHandle) = 0;

	virtual bool createTexture(void* _data, int _width, int _height, int _channels, TextureHandle& _outTextureHandle) = 0;
	virtual void destroyTexture(TextureHandle& _inTextureHandle) = 0;

	virtual bool createShader(ShaderType _type, const char* _source, size_t _sourceSize, ShaderHandle& _outShaderHandle) = 0;
	virtual void destroyShader(ShaderHandle& _shaderHandle) = 0;

	virtual bool createShaderProgram(ShaderHandle* _shaderHandles, u16 _shaderHandleCount, ShaderProgramHandle& _outShaderProgramHandle) = 0;
	virtual void destroyShaderProgram(ShaderProgramHandle& _shaderProgramHandle) = 0;

	virtual void drawMesh(const Matrix4& _transform, const Vertex* _vertices, u32 _verticesCount, const u32* _indices, u32 _indicesCount, const TextureHandle& _textureHandle) = 0;
	virtual void drawText(const Matrix4& _transform, const FontResource* _font, const char* _text) = 0;

	virtual void initIm3d() = 0;
	virtual void shutdownIm3d() = 0;
	virtual void drawIm3d(const Im3d::DrawList* _drawLists, u32 _drawListCount) = 0;

	void setViewProjectionMatrix(const Matrix4& _view, const Matrix4& _proj);

//private:
	Matrix4 m_viewMatrix = Matrix4::IDENTITY;
	Matrix4 m_projMatrix = Matrix4::IDENTITY;

};

} // namespace yae