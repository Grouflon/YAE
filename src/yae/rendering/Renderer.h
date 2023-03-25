#pragma once

#include <yae/types.h>
#include <yae/rendering/render_types.h>
#include <yae/math_types.h>
#include <yae/containers/HashMap.h>


typedef struct GLFWwindow GLFWwindow;
struct ImGuiContext;

namespace Im3d
{
struct Context;
struct DrawList;
}

namespace yae {

class FontFile;
class Mesh;
class Shader;
class ShaderProgram;
class Texture;
class RenderScene;
class RenderCamera;
class RenderTarget;

struct YAE_API DrawCommand
{
	PrimitiveMode primitiveMode;
	Matrix4 transform;
	u32 indexOffset;
	u32 elementCount;
	TextureHandle textureId;
};

class YAE_API RenderScene
{
public:
	RenderScene(const char* _name);
	~RenderScene();

	void addCamera(RenderCamera* _camera);
	void removeCamera(RenderCamera* _camera);

//private:
	char m_name[128] = {};
	DataArray<RenderCamera*> m_cameras;
	HashMap<ShaderProgramHandle, DataArray<DrawCommand>> m_drawCommands;
	Im3d::Context* m_im3d = nullptr;
};

class YAE_API RenderCamera
{
public:
	RenderCamera(const char* _name);
	~RenderCamera();

	Matrix4 computeViewMatrix() const;
	Matrix4 computeProjectionMatrix() const;
	Matrix4 computeViewProjectionMatrix() const;
	Vector2 getViewportSize() const;

	Vector3 project(const Vector3& _worldPosition) const;
	void unproject(const Vector2& _screenPosition, Vector3& _outRayOrigin, Vector3& _outRayDirection) const;

	Vector3 position = Vector3::ZERO;
	Quaternion rotation = Quaternion::IDENTITY;
	float fov = 45.f;
	float nearPlane = .1f;
	float farPlane = 100.f;
	RenderTarget* renderTarget = nullptr;
	Vector4 clearColor = Vector4(0.f, 0.f, 0.f, 1.f);

//private:
	char m_name[128] = {};
	RenderScene* m_scene = nullptr;
};

class YAE_API RenderTarget
{
public:
	bool fullScreen;

//private:
	u32 m_width;
	u32 m_height;
	FrameBufferHandle m_frameBuffer;
	TextureHandle m_renderTexture;
	TextureHandle m_depthTexture;
};

class YAE_API Renderer
{
public:
	virtual ~Renderer() {}

	bool init(GLFWwindow* _window);
	void shutdown();

	virtual void waitIdle() = 0;

	virtual RendererType getType() const = 0;

	virtual void hintWindow() {}

	void beginFrame();
  	void render();
  	void endFrame();

	Vector2 getFrameBufferSize() const;
	void notifyFrameBufferResized(int _width, int _height);

	virtual bool createTexture(const void* _data, int _width, int _height, int _channels, TextureHandle& _outTextureHandle) = 0;
	virtual void destroyTexture(TextureHandle& _inTextureHandle) = 0;

	RenderTarget* createRenderTarget(bool _fullScreen = true, u32 _width = 0, u32 _height = 0);
	void destroyRenderTarget(RenderTarget* _renderTarget);
	void resizeRenderTarget(RenderTarget* _renderTarget, u32 _width, u32 _height);

	virtual bool createShader(ShaderType _type, const char* _source, size_t _sourceSize, ShaderHandle& _outShaderHandle) = 0;
	virtual void destroyShader(ShaderHandle& _shaderHandle) = 0;

	virtual bool createShaderProgram(ShaderHandle* _shaderHandles, u16 _shaderHandleCount, ShaderProgramHandle& _outShaderProgramHandle) = 0;
	virtual void destroyShaderProgram(ShaderProgramHandle& _shaderProgramHandle) = 0;

	void drawMesh(const Matrix4& _transform, const Mesh* _mesh, const ShaderProgram* _shaderProgram, const Texture* _texture);
	void drawMesh(const Matrix4& _transform, const Vertex* _vertices, u32 _verticesCount, const u32* _indices, u32 _indicesCount, PrimitiveMode _primitiveMode, const ShaderProgramHandle& _shader, const TextureHandle& _texture);
	void drawText(const Matrix4& _transform, const FontFile* _font, const char* _text);

	RenderScene* createScene(const char* _sceneName);
	void destroyScene(RenderScene* _scene);
	void destroyScene(const char* _sceneName);
	RenderScene* getScene(const char* _sceneName) const;
	void pushScene(const char* _sceneName);
	void popScene();

	RenderCamera* createCamera(const char* _cameraName);
	void destroyCamera(RenderCamera* _camera);
	void destroyCamera(const char* _cameraName);
	RenderCamera* getCamera(const char* _cameraName) const;

//private:
	virtual bool _init() = 0;
	virtual void _shutdown() = 0;

	virtual bool _initImGui() = 0;
	virtual void _shutdownImGui() = 0;
	virtual void _renderImGui() = 0;

	virtual bool _initIm3d() = 0;
	virtual void _shutdownIm3d() = 0;
	virtual void _renderIm3d(const RenderCamera* _camera) = 0;

	virtual void _initRenderTarget(RenderTarget& _renderTarget) = 0;
	virtual void _shutdownRenderTarget(RenderTarget& _renderTarget) = 0;
	virtual void _resizeRenderTarget(RenderTarget& _renderTarget) = 0;

	virtual void _beginFrame() = 0;
	virtual void _beginRender() = 0;
	virtual void _renderCamera(const RenderCamera* _camera) = 0;
	virtual void _endRender() = 0;
	virtual void _endFrame() = 0;

	RenderScene* _getCurrentScene() const;

	GLFWwindow* m_window = nullptr;
	Im3d::Context* m_emptyIm3dContext = nullptr;

	ShaderProgram* m_fontShader = nullptr;
	Mesh* m_quad = nullptr;

	DataArray<Vertex> m_vertices;
	DataArray<u32> m_indices;
	HashMap<StringHash, RenderScene*> m_scenes;
	HashMap<StringHash, RenderCamera*> m_cameras;
	DataArray<RenderTarget*> m_renderTargets;
	DataArray<RenderTarget*> m_renderTargetsPendingDestruction;

	DataArray<RenderScene*> m_sceneStack;
};

} // namespace yae