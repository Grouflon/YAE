#pragma once

#include <yae/types.h>
#include <yae/time.h>
#include <yae/math_types.h>
#include <yae/containers/HashMap.h>

#include <SDL_events.h>

struct SDL_Window;
struct ImGuiContext;

namespace yae {

class Renderer;
class InputSystem;
class Serializer;

class YAE_API Application
{
public:
	Application(const char* _name, u32 _width, u32 _height);
	~Application();

	void init(char** _args, int _argCount);
	void shutdown();
	void pushEvent(const SDL_Event& _event);
	bool doFrame();

	void run();

	void requestExit();
	bool isRunning() const;

	const char* getName() const;

	InputSystem& input() const;
	Renderer& renderer() const;
	ResourceManager& resourceManager() const;

	void* getUserData(const char* _name) const;
	void setUserData(const char* _name, void* _userData);

	void loadSettings();
	void saveSettings();

	float getDeltaTime() const;
	float getTime() const;

	void setWindowSize(i32 _width, i32 _height);
	void setWindowSize(const Vector2& _size);
	void getWindowSize(i32* _outWidth, i32* _outHeight) const;
	Vector2 getWindowSize() const;

	void setWindowPosition(i32 _x, i32 _y);
	void setWindowPosition(const Vector2& _position);
	void getWindowPosition(i32* _outX, i32* _outY) const;
	Vector2 getWindowPosition() const;

//private:

	void _requestSaveSettings();

	String m_name;
	u32 m_baseWidth = 0;
	u32 m_baseHeight = 0;
	bool m_isRunning = true;

	ResourceManager* m_resourceManager = nullptr;
	InputSystem* m_inputSystem = nullptr;
	Renderer* m_renderer = nullptr;
	ImGuiContext* m_imguiContext = nullptr;

	SDL_Window* m_window = nullptr;
	DataArray<SDL_Event> m_events;

	Clock m_clock;
	float m_dt = 0.f;
	float m_time = 0.f;

	HashMap<StringHash, void*> m_userData;

	bool m_saveSettingsRequested = false;
};

} // namespace yae
