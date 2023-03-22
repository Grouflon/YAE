#include "Renderer.h"

#include <yae/Application.h>
#include <yae/input.h>
#include <yae/Mesh.h>
#include <yae/string.h>
#include <yae/math_3d.h>
#include <yae/resources/FontResource.h>

#include <GLFW/glfw3.h>
#include <im3d/im3d.h>
#include <imgui/imgui.h>

namespace yae {

RenderScene::RenderScene(const char* _name)
{
	string::safeCopyToBuffer(m_name, _name, countof(m_name));

	m_im3d = toolAllocator().create<Im3d::Context>();
}

RenderScene::~RenderScene()
{
	toolAllocator().destroy(m_im3d);
	m_im3d = nullptr;

	for (RenderCamera* camera : m_cameras)
	{
		camera->m_scene = nullptr;
	}
}

void RenderScene::addCamera(RenderCamera* _camera)
{
	YAE_ASSERT(_camera != nullptr);
	m_cameras.push_back(_camera);
	_camera->m_scene = this;
}

void RenderScene::removeCamera(RenderCamera* _camera)
{
	YAE_ASSERT(_camera != nullptr);
	YAE_ASSERT(_camera->m_scene == this);
	YAE_ASSERT(m_cameras.find(_camera) != m_cameras.end());

	m_cameras.erase(m_cameras.find(_camera));
	_camera->m_scene = nullptr;
}

RenderCamera::RenderCamera(const char* _name)
{
	string::safeCopyToBuffer(m_name, _name, countof(m_name));
}

RenderCamera::~RenderCamera()
{
	if (m_scene != nullptr)
	{
		m_scene->removeCamera(this);
	}
}

Matrix4 RenderCamera::computeViewMatrix() const
{
	Matrix4 cameraTransform = Matrix4::FromTransform(position, rotation, Vector3::ONE);
	return math::inverse(cameraTransform);
}

Matrix4 RenderCamera::computeProjectionMatrix() const
{
	Vector2 viewportSize = getViewportSize();
	YAE_ASSERT(!math::isZero(viewportSize));
	float radianFov = fov * D2R;
	float aspectRatio = viewportSize.x / viewportSize.y;
	return Matrix4::FromPerspective(radianFov, aspectRatio, nearPlane, farPlane);
}

Matrix4 RenderCamera::computeViewProjectionMatrix() const
{
	Matrix4 view = computeViewMatrix();
	Matrix4 projection = computeProjectionMatrix();

	return projection * view;
}

Vector2 RenderCamera::getViewportSize() const
{
	Vector2 viewportSize;
	if (renderTarget != nullptr)
	{
		viewportSize.x = renderTarget->m_width;
		viewportSize.y = renderTarget->m_height;
	}
	else
	{
		viewportSize = renderer().getFrameBufferSize();
	}
	return viewportSize;
}

Vector3 RenderCamera::project(const Vector3& _worldPosition) const
{
	Matrix4 view = computeViewMatrix();
	Matrix4 projection = computeProjectionMatrix();
	Vector2 viewportSize = getViewportSize();
	return math::project(_worldPosition, view, projection, Vector4(0.f, 0.f, viewportSize.x, viewportSize.y));
}

void RenderCamera::unproject(const Vector2& _screenPosition, Vector3& _outRayOrigin, Vector3& _outRayDirection) const
{
	Matrix4 view = computeViewMatrix();
	Matrix4 projection = computeProjectionMatrix();
	Vector2 viewportSize = getViewportSize();
	math::unproject(_screenPosition, view, projection, Vector4(0.f, 0.f, viewportSize.x, viewportSize.y), _outRayOrigin, _outRayDirection);
}

bool Renderer::init(GLFWwindow* _window)
{
	YAE_CAPTURE_FUNCTION();

	m_window = _window;

	if (!_init())
		return false;

	if (!_initImGui())
		return false;

	m_emptyIm3dContext = toolAllocator().create<Im3d::Context>();
	Im3d::SetContext(*m_emptyIm3dContext);
	if (!_initIm3d())
		return false;

	return true;
}

void Renderer::shutdown()
{
	YAE_CAPTURE_FUNCTION();

	_shutdownImGui();

	_shutdownIm3d();
	toolAllocator().destroy(m_emptyIm3dContext);
	m_emptyIm3dContext = nullptr;

	_shutdown();

	// Let's clear everything, even if the user leaks things
	for (auto& pair : m_cameras)
	{
		defaultAllocator().destroy(pair.value);
	}
	m_cameras.clear();

	for (auto& pair : m_scenes)
	{
		defaultAllocator().destroy(pair.value);
	}
	m_scenes.clear();

	for (RenderTarget* renderTarget : m_renderTargets)
	{
		_shutdownRenderTarget(*renderTarget);
		defaultAllocator().destroy(renderTarget);
	}
	m_renderTargets.clear();
}

void Renderer::beginFrame()
{
	_beginFrame();

	// Prepare all scenes
	for (const auto& pair : m_scenes)
	{
		RenderScene* scene = pair.value;

		if (scene->m_cameras.size() == 0)
			continue;

		// Im3d
		RenderCamera* im3dCamera = scene->m_cameras[0];
		Im3d::SetContext(*scene->m_im3d);
		Im3d::AppData& ad = Im3d::GetAppData();

		// View
		Vector2 viewportSize = im3dCamera->getViewportSize();
		Matrix4 worldMatrix = Matrix4::FromTransform(im3dCamera->position, im3dCamera->rotation, Vector3::ONE);
		Matrix4 viewMatrix = math::inverse(worldMatrix);
		Matrix4 projectionMatrix = im3dCamera->computeProjectionMatrix();

		
		// Set cull frustum planes. This is only required if IM3D_CULL_GIZMOS or IM3D_CULL_PRIMTIIVES is enable via
		// im3d_config.h, or if any of the IsVisible() functions are called.
		Matrix4 viewProj = projectionMatrix * viewMatrix;
		ad.setCullFrustum(viewProj, true);

		ad.m_viewportSize  = viewportSize;
		ad.m_viewOrigin    = im3dCamera->position; // for VR use the head position
		ad.m_viewDirection = math::forward(im3dCamera->rotation);
		ad.m_worldUp       = Vector3::UP; // used internally for generating orthonormal bases
		ad.m_projOrtho     = false; // TODO
		
		// m_projScaleY controls how gizmos are scaled in world space to maintain a constant screen height
		ad.m_projScaleY = ad.m_projOrtho
			? 2.0f / projectionMatrix[1][1] // use far plane height for an ortho projection
			: tanf(im3dCamera->fov * 0.5f) * 2.0f // or vertical fov for a perspective projection
			;  

		// Input
		const bool inputEnabled =
			(im3dCamera->renderTarget == nullptr || im3dCamera->renderTarget->fullScreen) &&
			!ImGui::GetIO().WantCaptureMouse;
		ad.m_deltaTime = app().getDeltaTime();

		// Fill the key state array; using GetAsyncKeyState here but this could equally well be done via the window proc.
		// All key states have an equivalent (and more descriptive) 'Action_' enum.
		//ad.m_keyDown[Im3d::Mouse_Left/*Im3d::Action_Select*/] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
		ad.m_keyDown[Im3d::Action_Select] = inputEnabled && input().isMouseButtonDown(0);

		// The following key states control which gizmo to use for the generic Gizmo() function. Here using the left ctrl
	 	// key as an additional predicate.
		//bool ctrlDown = (GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0;
		//ad.m_keyDown[Im3d::Key_L/*Action_GizmoLocal*/]       = ctrlDown && (GetAsyncKeyState(0x4c) & 0x8000) != 0;
		//ad.m_keyDown[Im3d::Key_T/*Action_GizmoTranslation*/] = ctrlDown && (GetAsyncKeyState(0x54) & 0x8000) != 0;
		//ad.m_keyDown[Im3d::Key_R/*Action_GizmoRotation*/]    = ctrlDown && (GetAsyncKeyState(0x52) & 0x8000) != 0;
		//ad.m_keyDown[Im3d::Key_S/*Action_GizmoScale*/]       = ctrlDown && (GetAsyncKeyState(0x53) & 0x8000) != 0;

		ad.m_keyDown[Im3d::Key_T/*Action_GizmoTranslation*/] = inputEnabled && input().isKeyDown(GLFW_KEY_1);
		ad.m_keyDown[Im3d::Key_R/*Action_GizmoRotation*/]    = inputEnabled && input().isKeyDown(GLFW_KEY_2);
		ad.m_keyDown[Im3d::Key_S/*Action_GizmoScale*/]       = inputEnabled && input().isKeyDown(GLFW_KEY_3);


		// Enable gizmo snapping by setting the translation/rotation/scale increments to be > 0
		//ad.m_snapTranslation = ctrlDown ? 0.5f : 0.0f;
		//ad.m_snapRotation    = ctrlDown ? Im3d::Radians(30.0f) : 0.0f;
		//ad.m_snapScale       = ctrlDown ? 0.5f : 0.0f;

		ad.m_snapTranslation = 0.0f;
		ad.m_snapRotation    = 0.0f;
		ad.m_snapScale       = 0.0f;

		// World space cursor ray from mouse position; for VR this might be the position/orientation of the HMD or a tracked controller.
		Vector2 cursorPos = input().getMousePosition();
		cursorPos.y = viewportSize.y - cursorPos.y; // window origin is top-left, ndc is bottom-left
		Vector3 rayOrigin, rayDirection;
		im3dCamera->unproject(cursorPos, rayOrigin, rayDirection);

		ad.m_cursorRayOrigin = rayOrigin;
		ad.m_cursorRayDirection = rayDirection;

		if (!inputEnabled)
		{
			// TODO: This does not work and keeps previously hovered states. we need to find a way to force all hovers out when input is disabled
			ad.m_cursorRayDirection = ad.m_cursorRayDirection * -1.f;
		}

		// NOTE: I think that Im3d will always have one frame lag if we want to be able to call it whenever we want. that's unfortunate
		Im3d::NewFrame();
	}
	Im3d::SetContext(*m_emptyIm3dContext);
}

void Renderer::render()
{
	_beginRender();

	// Prepare all scenes
	for (const auto& pair : m_scenes)
	{
		RenderScene* scene = pair.value;
		if (scene->m_cameras.size() == 0)
			continue;

		// Im3d
		Im3d::SetContext(*scene->m_im3d);
		RenderCamera* im3dCamera = scene->m_cameras[0];
		if (im3dCamera)
		{
			Im3d::EndFrame();
		}
		Im3d::SetContext(*m_emptyIm3dContext);
	}

	// Render all cameras
	for (const auto& pair : m_cameras)
	{
		RenderCamera* camera = pair.value;
		if (camera->m_scene == nullptr)
			continue;

		_renderCamera(camera);

		Im3d::SetContext(*camera->m_scene->m_im3d);
		_renderIm3d(camera);
		Im3d::SetContext(*m_emptyIm3dContext);
	}

	// Render ImGui
	_renderImGui();

	_endRender();
}

void Renderer::endFrame()
{
	_endFrame();

	// Clear all commands
	for (const auto& pair : m_scenes)
	{
		RenderScene* scene = pair.value;
		scene->m_drawCommands.clear();
	}
	m_vertices.clear();
	m_indices.clear();

	// Clear objects pending destruction
	for (RenderTarget* renderTarget : m_renderTargetsPendingDestruction)
	{
		_shutdownRenderTarget(*renderTarget);

		m_renderTargets.erase(m_renderTargets.find(renderTarget));
		defaultAllocator().destroy(renderTarget);
	}
	m_renderTargetsPendingDestruction.clear();
}

Vector2 Renderer::getFrameBufferSize() const 
{
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);

    return Vector2(width, height);
}

void Renderer::notifyFrameBufferResized(int _width, int _height)
{
	Vector2 frameBufferSize = getFrameBufferSize();
	for (RenderTarget* renderTarget : m_renderTargets)
	{
		if (renderTarget->fullScreen)
		{
			renderTarget->m_width = frameBufferSize.x;
			renderTarget->m_height = frameBufferSize.y;
			_resizeRenderTarget(*renderTarget);
		}
	}
}

RenderTarget* Renderer::createRenderTarget(bool _fullScreen, u32 _width, u32 _height)
{
	RenderTarget* renderTarget = defaultAllocator().create<RenderTarget>();
	m_renderTargets.push_back(renderTarget);

	renderTarget->fullScreen = _fullScreen;
	if (renderTarget->fullScreen)
	{
		Vector2 frameBufferSize = getFrameBufferSize();
		renderTarget->m_width = frameBufferSize.x;
		renderTarget->m_height = frameBufferSize.y;
	}
	else
	{
		renderTarget->m_width = _width;
		renderTarget->m_height = _height;
	}

	_initRenderTarget(*renderTarget);

	return renderTarget;
}

void Renderer::destroyRenderTarget(RenderTarget* _renderTarget)
{
	YAE_ASSERT(_renderTarget != nullptr);
	YAE_ASSERT(m_renderTargets.find(_renderTarget) != m_renderTargets.end());

	m_renderTargetsPendingDestruction.push_back(_renderTarget);
}

void Renderer::resizeRenderTarget(RenderTarget* _renderTarget, u32 _width, u32 _height)
{
	YAE_ASSERT(_renderTarget != nullptr);
	YAE_ASSERT(!_renderTarget->fullScreen);

	if (_renderTarget->m_width == _width && _renderTarget->m_height == _height)
		return;

	_renderTarget->m_width = _width;
	_renderTarget->m_height = _height;

	_resizeRenderTarget(*_renderTarget);
}

void Renderer::drawMesh(const Matrix4& _transform, const Mesh& _mesh, const ShaderProgramHandle& _shader, const TextureHandle& _texture)
{
	drawMesh(
		_transform,
		_mesh.vertices.data(), _mesh.vertices.size(),
		_mesh.indices.data(), _mesh.indices.size(),
		_shader,
		_texture
	);
}


void Renderer::drawMesh(const Matrix4& _transform, const Vertex* _vertices, u32 _verticesCount, const u32* _indices, u32 _indicesCount, const ShaderProgramHandle& _shader, const TextureHandle& _texture)
{
	RenderScene* scene = _getCurrentScene();

	//u32 shader = (u32)_shader;
	u32 shader = (u32)m_shader;
	DataArray<DrawCommand>* commandArray = scene->m_drawCommands.get(shader);
	if (commandArray == nullptr)
	{
		commandArray = &scene->m_drawCommands.set(shader, DataArray<DrawCommand>());
	}

	u32 baseIndex = m_vertices.size();
	u32 startIndex = m_indices.size();

	DrawCommand command;
	command.transform = _transform;
	command.indexOffset = startIndex;
	command.elementCount = _indicesCount;
	command.textureId = _texture;
	commandArray->push_back(command);

	m_vertices.push_back(_vertices, _verticesCount);

	m_indices.resize(m_indices.size() + _indicesCount);
	for (u32 i = 0; i < _indicesCount; ++i)
	{
		m_indices[startIndex + i] = baseIndex + _indices[i];
	}
}


void Renderer::drawText(const Matrix4& _transform, const FontResource* _font, const char* _text)
{
	RenderScene* scene = _getCurrentScene();

	u32 shader = (u32)m_fontShader;
	DataArray<DrawCommand>* commandArray = scene->m_drawCommands.get(shader);
	if (commandArray == nullptr)
	{
		commandArray = &scene->m_drawCommands.set(shader, DataArray<DrawCommand>());
	}

	u32 indicesStart = m_indices.size();
	u32 verticesStart = m_vertices.size();
	u32 textLength = strlen(_text);

	m_vertices.resize(m_vertices.size() + textLength * 4);
	m_indices.resize(m_indices.size() + textLength * 6);

	float xPos = 0.f;
	float yPos = 0.f;
	Vertex vertices[4];
	glm::vec3 _color(1.f, 1.f, 1.f);
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
	command.textureId = _font->m_fontTexture;
	commandArray->push_back(command);
}

RenderScene* Renderer::createScene(const char* _sceneName)
{
	StringHash nameHash(_sceneName);
	YAE_ASSERT(m_scenes.get(nameHash) == nullptr);

	RenderScene* scene = defaultAllocator().create<RenderScene>(_sceneName);
	m_scenes.set(nameHash, scene);
	return scene;
}

void Renderer::destroyScene(RenderScene* _scene)
{
	YAE_ASSERT(_scene != nullptr);

	StringHash nameHash(_scene->m_name);
	YAE_ASSERT(m_scenes.get(nameHash) != nullptr);

	m_scenes.remove(nameHash);
	defaultAllocator().destroy(_scene);
}

void Renderer::destroyScene(const char* _sceneName)
{
	destroyScene(getScene(_sceneName));
}

RenderScene* Renderer::getScene(const char* _sceneName) const
{
	StringHash nameHash(_sceneName);
	RenderScene*const* scenePtr = m_scenes.get(nameHash);
	return scenePtr != nullptr ? *scenePtr : nullptr;
}

void Renderer::pushScene(const char* _sceneName)
{
	RenderScene* scene = getScene(_sceneName);
	YAE_ASSERT(scene != nullptr);

	m_sceneStack.push_back(scene);
	Im3d::SetContext(*scene->m_im3d);
}

void Renderer::popScene()
{
	YAE_ASSERT(m_sceneStack.size() > 0);
	m_sceneStack.pop_back();

	Im3d::SetContext(m_sceneStack.size() > 0 ? *m_sceneStack.back()->m_im3d : *m_emptyIm3dContext);
}

RenderCamera* Renderer::createCamera(const char* _cameraName)
{
	StringHash nameHash(_cameraName);
	YAE_ASSERT(m_cameras.get(nameHash) == nullptr);

	RenderCamera* camera = defaultAllocator().create<RenderCamera>(_cameraName);
	m_cameras.set(nameHash, camera);
	return camera;
}

void Renderer::destroyCamera(RenderCamera* _camera)
{
	YAE_ASSERT(_camera != nullptr);

	StringHash nameHash(_camera->m_name);
	YAE_ASSERT(m_cameras.get(nameHash) != nullptr);

	m_cameras.remove(nameHash);
	defaultAllocator().destroy(_camera);
}

void Renderer::destroyCamera(const char* _cameraName)
{
	destroyCamera(getCamera(_cameraName));
}


RenderCamera* Renderer::getCamera(const char* _cameraName) const
{
	StringHash nameHash(_cameraName);
	RenderCamera*const* cameraPtr = m_cameras.get(nameHash);
	return cameraPtr != nullptr ? *cameraPtr : nullptr;
}

RenderScene* Renderer::_getCurrentScene() const
{
	YAE_ASSERT(m_sceneStack.size() > 0);
	return m_sceneStack.back();
}

} // namespace yae
