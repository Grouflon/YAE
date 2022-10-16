#pragma once

#include <yae/types.h>
#include <yae/rendering/render_types.h>

struct GLFWwindow;
struct ImGuiContext;

namespace yae {

class Renderer;

class YAE_API ImGuiSystem
{
public:
	void init(GLFWwindow* _window, Renderer* _renderer);
	void shutdown();

	void newFrame();
	void render(FrameHandle _frameHandle);

//private:
	ImGuiContext* m_imguiContext = nullptr;
	Renderer* m_renderer = nullptr;
};

} // namespace yae
