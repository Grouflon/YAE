#include <game_module.h>

#include <profiling.h>
#include <platform.h>

namespace yae {

const char* GAME_DLL_PATH = "./data/code/yaeGame.dll";
const char* TMP_GAME_DLL_PATH = "./data/code/yaeGame_temp.dll";

typedef void (*GameFunctionPtr)();

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
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(s_gameAPI.libraryHandle == nullptr);

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
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(s_gameAPI.libraryHandle != nullptr);
	
	s_gameAPI.onLibraryUnloaded();
	
	platform::unloadDynamicLibrary(s_gameAPI.libraryHandle);
	s_gameAPI = {};
}

void watchGameAPI()
{
	YAE_CAPTURE_FUNCTION();

	u64 lastWriteTime = platform::getFileLastWriteTime(GAME_DLL_PATH);
	if (lastWriteTime != s_gameAPI.lastWriteTime)
	{
		unloadGameAPI();
		loadGameAPI();
	}
}

void initGame()
{
	YAE_ASSERT(s_gameAPI.libraryHandle != nullptr);

	s_gameAPI.gameInit();
}

void updateGame()
{
	YAE_ASSERT(s_gameAPI.libraryHandle != nullptr);

	s_gameAPI.gameUpdate();
}

void shutdownGame()
{
	YAE_ASSERT(s_gameAPI.libraryHandle != nullptr);

	s_gameAPI.gameShutdown();
}

} // namespace yae
