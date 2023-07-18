#pragma once

#include <yae/types.h>
#include <yae/rendering/Renderer.h>
#include <yae/containers/HashMap.h>

#include <im3d/im3d.h>
namespace yae {

class ShaderResource;

class YAE_API OpenGLRenderer : public Renderer
{
public:
	virtual RendererType getType() const override { return RendererType::OpenGL; }

	virtual void hintWindow() const override;
	virtual u32 getWindowFlags() const override;

	virtual void waitIdle() override;

	virtual bool createTexture(const void* _data, int _width, int _height, int _channels, TextureHandle& _outTextureHandle) override;
	virtual void applyTextureParameters(TextureHandle& _inTextureHandle, const TextureParameters& _parameters) override;
	virtual void destroyTexture(TextureHandle& _inTextureHandle) override;

	virtual bool createShader(ShaderType _type, const char* _source, size_t _sourceSize, ShaderHandle& _outShaderHandle) override;
	virtual void destroyShader(ShaderHandle& _shaderHandle) override;

	virtual bool createShaderProgram(ShaderHandle* _shaderHandles, u16 _shaderHandleCount, ShaderProgramHandle& _outShaderProgramHandle) override;
	virtual void destroyShaderProgram(ShaderProgramHandle& _shaderProgramHandle) override;

	const char* getShaderVersion() const;

//private:
	virtual bool _init() override;
	virtual void _shutdown() override;

	virtual bool _initImGui() override;
	virtual void _renderImGui() override;
	virtual void _shutdownImGui() override;

	virtual bool _initIm3d() override;
	virtual void _shutdownIm3d() override;
	virtual void _renderIm3d(const RenderCamera* _camera) override;

	virtual void _initRenderTarget(RenderTarget& _renderTarget) override;
	virtual void _resizeRenderTarget(RenderTarget& _renderTarget) override;
	virtual void _shutdownRenderTarget(RenderTarget& _renderTarget) override;

	virtual void _beginFrame() override;
	virtual void _beginRender() override;
	virtual void _renderCamera(const RenderCamera* _camera) override;
	virtual void _endRender() override;
	virtual void _endFrame() override;

	Matrix4 _computeFixedViewProjectionMatrix(const RenderCamera* _camera) const;

	void* m_glContext = nullptr;

	u32 m_vao = 0;
	u32 m_vertexBufferObject = 0;
	u32 m_indexBufferObject = 0;

	u32 m_im3dVertexArray = 0;
	u32 m_im3dVertexBuffer = 0;
	ShaderProgramHandle m_im3dShaderPoints = 0;
	ShaderProgramHandle m_im3dShaderLines = 0;
	ShaderProgramHandle m_im3dShaderTriangles = 0;

	u32 m_quadVertexArray = 0;
	u32 m_quadVertexBuffer = 0;
};

} // namespace yae
