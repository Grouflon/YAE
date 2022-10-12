#pragma once

#include <yae/types.h>
#include <yae/rendering/render_types.h>
#include <yae/math_types.h>

namespace Im3d
{
	struct Context;
}

namespace yae {

struct Im3dCamera
{
	Vector3 position;
	Vector3 direction;
	Matrix4 view;
	Matrix4 projection;
	float fov; // in radians
	bool orthographic;
};

class Im3dSystem
{
public:
	void init();
	void shutdown();

	void newFrame(float _dt, const Im3dCamera& _camera);
	void render(FrameHandle _frameHandle);

// private:
	Im3d::Context* m_im3d = nullptr;
};

} // namespace yae
