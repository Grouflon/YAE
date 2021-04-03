#include "Application.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>

#include <platform.h>
#include <log.h>
#include <vulkan/VulkanRenderer.h>

typedef void (*GameFunctionPtr)();

namespace yae {

const char* GAME_DLL_PATH = "./data/code/yaeGame.dll";
const char* TMP_GAME_DLL_PATH = "./data/code/yaeGame_temp.dll";

struct GameAPI
{
	u64 lastWriteTime = 0;
	void* libraryHandle = nullptr;
	GameFunctionPtr gameInit = nullptr;
	GameFunctionPtr gameUpdate = nullptr;
	GameFunctionPtr gameShutdown = nullptr;
	GameFunctionPtr onLibraryLoaded = nullptr;
	GameFunctionPtr onLibraryUnloaded = nullptr;
} s_gameAPI;

void loadGameAPI()
{
	bool ret = platform::duplicateFile(GAME_DLL_PATH, TMP_GAME_DLL_PATH);
	YAE_ASSERT(ret);
	s_gameAPI.lastWriteTime = platform::getFileLastWriteTime(GAME_DLL_PATH);
	s_gameAPI.libraryHandle = platform::loadDynamicLibrary(TMP_GAME_DLL_PATH);
	YAE_ASSERT(s_gameAPI.libraryHandle);

	s_gameAPI.gameInit = (GameFunctionPtr)platform::getProcedureAddress(s_gameAPI.libraryHandle, "initGame");
	s_gameAPI.gameUpdate = (GameFunctionPtr)platform::getProcedureAddress(s_gameAPI.libraryHandle, "updateGame");
	s_gameAPI.gameShutdown = (GameFunctionPtr)platform::getProcedureAddress(s_gameAPI.libraryHandle, "shutdownGame");
	s_gameAPI.onLibraryLoaded = (GameFunctionPtr)platform::getProcedureAddress(s_gameAPI.libraryHandle, "onLibraryLoaded");
	s_gameAPI.onLibraryUnloaded = (GameFunctionPtr)platform::getProcedureAddress(s_gameAPI.libraryHandle, "onLibraryUnloaded");

	YAE_ASSERT(s_gameAPI.gameInit);
	YAE_ASSERT(s_gameAPI.gameUpdate);
	YAE_ASSERT(s_gameAPI.gameShutdown);

	s_gameAPI.onLibraryLoaded();
}

void unloadGameAPI()
{
	s_gameAPI.onLibraryUnloaded();
	
	platform::unloadDynamicLibrary(s_gameAPI.libraryHandle);
	s_gameAPI = {};
}

void watchGameAPI()
{
	u64 lastWriteTime = platform::getFileLastWriteTime(GAME_DLL_PATH);
	if (lastWriteTime != s_gameAPI.lastWriteTime)
	{
		unloadGameAPI();
		loadGameAPI();
	}
}

void onGlfwKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_R && action == GLFW_PRESS && mods & GLFW_MOD_CONTROL)
	{
		unloadGameAPI();
		loadGameAPI();
	}
}

void Application::init(const char* _name, u32 _width, u32 _height, char** _args, int _arg_count)
{
	m_name = _name;

	m_exePath = Path(_args[0]);
	YAE_LOG(m_exePath.c_str());

	String workingDirectory = platform::getWorkingDirectory();
	Path workingDirectoryPath(workingDirectory.c_str());
	YAE_LOG(workingDirectoryPath.c_str());

	// Init GLFW
	{
		int result = glfwInit();
		YAE_ASSERT(result == GLFW_TRUE);
		YAE_VERBOSE_CAT("glfw", "Initialized glfw");
	}

	// Init Window
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Disable OpenGL context creation
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	m_window = glfwCreateWindow(_width, _height, m_name.c_str(), nullptr, nullptr);
	glfwSetKeyCallback(m_window, &onGlfwKeyEvent);

	YAE_ASSERT(m_window);
	YAE_VERBOSE_CAT("glfw", "Created glfw window");

	// Init Vulkan
	m_vulkanWrapper = new VulkanRenderer();

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif
	m_vulkanWrapper->init(m_window, enableValidationLayers);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	m_imgui = ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	{
		bool ret = ImGui_ImplGlfw_InitForVulkan(m_window, true);
		YAE_ASSERT(ret);
	}
	m_vulkanWrapper->initImGui();

	loadGameAPI();

	s_gameAPI.gameInit();
}

void Application::run()
{
	//static bool s_showDemoWindow = true;
	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();

		ImGui_ImplGlfw_NewFrame();
		m_vulkanWrapper->beginFrame();
		ImGui::NewFrame();

		s_gameAPI.gameUpdate();

		if (ImGui::BeginMainMenuBar())
	    {
	        if (ImGui::BeginMenu("File"))
	        {
	        	if (ImGui::MenuItem("Exit"))
	        	{
	        		glfwSetWindowShouldClose(m_window, true);
	        	}
	            ImGui::EndMenu();
	        }
	        ImGui::EndMainMenuBar();
	    }

		//ImGui::ShowDemoWindow(&s_showDemoWindow);

		m_vulkanWrapper->drawMesh();

		// Rendering
		ImGui::Render();
		ImDrawData* imguiDrawData = ImGui::GetDrawData();
		const bool isMinimized = (imguiDrawData->DisplaySize.x <= 0.0f || imguiDrawData->DisplaySize.y <= 0.0f);
		if (!isMinimized)
		{
			m_vulkanWrapper->drawImGui(imguiDrawData);
		}

		m_vulkanWrapper->endFrame();

		watchGameAPI();
	}
	m_vulkanWrapper->waitIdle();
}

void Application::shutdown()
{
	s_gameAPI.gameShutdown();
	unloadGameAPI();

	m_vulkanWrapper->shutdownImGui();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext(m_imgui);
	m_imgui = nullptr;

	m_vulkanWrapper->shutdown();
	delete m_vulkanWrapper;
	m_vulkanWrapper = nullptr;

	glfwDestroyWindow(m_window);
	m_window = nullptr;
	YAE_VERBOSE_CAT("glfw", "Destroyed glfw window");

	glfwTerminate();
	YAE_VERBOSE_CAT("glfw", "Terminated glfw");
}

} // namespace yae
