#include "Im3dSystem.h"

#include <yae/input.h>
#include <yae/rendering/Renderer.h>

#include <im3d/im3d.h>

namespace yae {

void Im3dSystem::init()
{
	m_im3d = toolAllocator().create<Im3d::Context>();
}

void Im3dSystem::shutdown()
{
	toolAllocator().destroy(m_im3d);
	m_im3d = nullptr;
}

void Im3dSystem::newFrame(float _dt, const Im3dCamera& _camera)
{
	YAE_CAPTURE_FUNCTION();

	Im3d::SetContext(*m_im3d);

	Im3d::AppData& ad = Im3d::GetAppData();

	Vector2 viewportSize = renderer().getFrameBufferSize();

	ad.m_deltaTime     = _dt;
	ad.m_viewportSize  = viewportSize;
	ad.m_viewOrigin    = _camera.position; // for VR use the head position
	ad.m_viewDirection = _camera.direction;
	ad.m_worldUp       = Vector3::UP; // used internally for generating orthonormal bases
	ad.m_projOrtho     = _camera.orthographic; 
	
 // m_projScaleY controls how gizmos are scaled in world space to maintain a constant screen height
	ad.m_projScaleY = _camera.orthographic
		? 2.0f / _camera.projection[1][1] // use far plane height for an ortho projection
		: tanf(_camera.fov * 0.5f) * 2.0f // or vertical fov for a perspective projection
		;  

 // World space cursor ray from mouse position; for VR this might be the position/orientation of the HMD or a tracked controller.
	Vector2 cursorPos = input().getMousePosition();
	cursorPos = (cursorPos / viewportSize) * 2.0f - 1.0f;
	cursorPos.y = -cursorPos.y; // window origin is top-left, ndc is bottom-left
	Vector4 rayOrigin, rayDirection;
	Matrix4 worldMatrix = inverse(_camera.view);
	if (_camera.orthographic)
	{
		rayOrigin.x  = cursorPos.x / _camera.projection[0][0];
		rayOrigin.y  = cursorPos.y / _camera.projection[1][1];
		rayOrigin.z  = 0.0f;
		rayOrigin.w = 1.0f;
		rayOrigin    = worldMatrix * rayOrigin;
		rayDirection = worldMatrix * Vector4(0.0f, 0.0f, -1.0f, 0.0f);
		 
	}
	else
	{
		rayOrigin = Vector4(ad.m_viewOrigin, 1.0f);
		rayDirection.x  = cursorPos.x / _camera.projection[0][0];
		rayDirection.y  = cursorPos.y / _camera.projection[1][1];
		rayDirection.z  = -1.0f;
		rayDirection.w  = 0.0f;
		rayDirection    = worldMatrix * normalize(rayDirection);
	}
	ad.m_cursorRayOrigin = rayOrigin.xyz();
	ad.m_cursorRayDirection = rayDirection.xyz();

 // Set cull frustum planes. This is only required if IM3D_CULL_GIZMOS or IM3D_CULL_PRIMTIIVES is enable via
 // im3d_config.h, or if any of the IsVisible() functions are called.
	Matrix4 viewProj = Matrix4(_camera.projection * _camera.view);
	ad.setCullFrustum(viewProj, true);

 // Fill the key state array; using GetAsyncKeyState here but this could equally well be done via the window proc.
 // All key states have an equivalent (and more descriptive) 'Action_' enum.
	//ad.m_keyDown[Im3d::Mouse_Left/*Im3d::Action_Select*/] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	ad.m_keyDown[Im3d::Action_Select] = input().isMouseButtonDown(0);

 // The following key states control which gizmo to use for the generic Gizmo() function. Here using the left ctrl
 // key as an additional predicate.
	//bool ctrlDown = (GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0;
	//ad.m_keyDown[Im3d::Key_L/*Action_GizmoLocal*/]       = ctrlDown && (GetAsyncKeyState(0x4c) & 0x8000) != 0;
	//ad.m_keyDown[Im3d::Key_T/*Action_GizmoTranslation*/] = ctrlDown && (GetAsyncKeyState(0x54) & 0x8000) != 0;
	//ad.m_keyDown[Im3d::Key_R/*Action_GizmoRotation*/]    = ctrlDown && (GetAsyncKeyState(0x52) & 0x8000) != 0;
	//ad.m_keyDown[Im3d::Key_S/*Action_GizmoScale*/]       = ctrlDown && (GetAsyncKeyState(0x53) & 0x8000) != 0;

 // Enable gizmo snapping by setting the translation/rotation/scale increments to be > 0
	//ad.m_snapTranslation = ctrlDown ? 0.5f : 0.0f;
	//ad.m_snapRotation    = ctrlDown ? Im3d::Radians(30.0f) : 0.0f;
	//ad.m_snapScale       = ctrlDown ? 0.5f : 0.0f;

	ad.m_snapTranslation = 0.0f;
	ad.m_snapRotation    = 0.0f;
	ad.m_snapScale       = 0.0f;

	Im3d::NewFrame();
}

void Im3dSystem::endFrame()
{
	YAE_CAPTURE_FUNCTION();

	Im3d::EndFrame();
}

void Im3dSystem::render(FrameHandle _frameHandle)
{
	YAE_CAPTURE_FUNCTION();

	renderer().drawIm3d(Im3d::GetDrawLists(), Im3d::GetDrawListCount());
}

} // namespace yae
