#pragma once

#include <yae/types.h>

#if YAE_IMPLEMENTS_RENDERER_OPENGL

#include <yae/Renderer.h>

namespace yae {

class YAELIB_API OpenGLRenderer : public Renderer
{
	MIRROR_CLASS(OpenGLRenderer)
	(
		MIRROR_PARENT(Renderer)
	);


};

} // namespace yae

#endif // YAE_IMPLEMENTS_RENDERER_OPENGL
