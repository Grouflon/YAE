#pragma once

#include <yae/types.h>
#include <yae/rendering/Renderer.h>
#include <yae/containers/HashMap.h>

namespace yae {

class ShaderResource;

class YAELIB_API OpenGLRenderer : public Renderer
{
public:
	virtual bool init(GLFWwindow* _window) override;
	virtual void shutdown() override;

	virtual RendererType getType() const override { return RendererType::OpenGL; }

	virtual void hintWindow() override;

	virtual FrameHandle beginFrame() override;
  	virtual void endFrame() override;
	virtual void waitIdle() override;

	virtual Vector2 getFrameBufferSize() const override;
	virtual void notifyFrameBufferResized(int _width, int _height) override;

	virtual void drawCommands(FrameHandle _frameHandle) override;

	virtual bool createTexture(void* _data, int _width, int _height, int _channels, TextureHandle& _outTextureHandle) override;
	virtual void destroyTexture(TextureHandle& _inTextureHandle) override;

	virtual bool createShader(ShaderType _type, const char* _source, size_t _sourceSize, ShaderHandle& _outShaderHandle) override;
	virtual void destroyShader(ShaderHandle& _shaderHandle) override;

	virtual bool createShaderProgram(ShaderHandle* _shaderHandles, u16 _shaderHandleCount, ShaderProgramHandle& _outShaderProgramHandle) override;
	virtual void destroyShaderProgram(ShaderProgramHandle& _shaderProgramHandle) override;

	virtual void drawMesh(const Matrix4& _transform, const Vertex* _vertices, u32 _verticesCount, const u32* _indices, u32 _indicesCount, const TextureHandle& _textureHandle) override;
	virtual void drawText(const Matrix4& _transform, const FontResource* _font, const char* _text) override;

	const char* getShaderVersion() const;

//private:
	GLFWwindow* m_window = nullptr;

	ShaderProgramHandle m_shader = nullptr;
	ShaderProgramHandle m_fontShader = nullptr;

	u32 m_vao = 0;
	u32 m_vertexBufferObject = 0;
	u32 m_indexBufferObject = 0;
	/*
	i32 m_uniformLocation_view = 0;
	i32 m_uniformLocation_proj = 0;
	i32 m_uniformLocation_model = 0;
	i32 m_uniformLocation_texSampler = 0;
	*/

	DataArray<Vertex> m_vertices;
	DataArray<u32> m_indices;
	struct DrawCommand
	{
		Matrix4 transform;
		u32 indexOffset;
		u32 elementCount;
		u32 textureId;
	};
	HashMap<u32, DataArray<DrawCommand>> m_drawCommands;
};

} // namespace yae
