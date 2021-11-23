#include "game_module.h"

#include <yae/platform.h>

namespace yae {

const char* dllSrcPath = "C:/Projects/CODE/Yae/lib/Win64_Debug/game.dll";
const char* symbolsSrcPath = "C:/Projects/CODE/Yae/lib/Win64_Debug/game.pdb";
const char* dllDstPath = "C:/Projects/CODE/Yae/bin/Win64_Debug/game_runtime.dll";
const char* symbolsDstPath = "C:/Projects/CODE/Yae/bin/Win64_Debug/game_runtime.pdb";

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

	YAE_VERIFY(platform::duplicateFile(dllSrcPath, dllDstPath));
	YAE_VERIFY(platform::duplicateFile(symbolsSrcPath, symbolsDstPath));

	s_gameAPI.libraryHandle = platform::loadDynamicLibrary(dllDstPath);
	YAE_ASSERT(s_gameAPI.libraryHandle);
	s_gameAPI.lastWriteTime = platform::getFileLastWriteTime(dllSrcPath);

	s_gameAPI.gameInit = (GameFunctionPtr)platform::getProcedureAddress(s_gameAPI.libraryHandle, "initGame");
	s_gameAPI.gameUpdate = (GameFunctionPtr)platform::getProcedureAddress(s_gameAPI.libraryHandle, "updateGame");
	s_gameAPI.gameShutdown = (GameFunctionPtr)platform::getProcedureAddress(s_gameAPI.libraryHandle, "shutdownGame");
	s_gameAPI.onLibraryLoaded = (GameFunctionPtr)platform::getProcedureAddress(s_gameAPI.libraryHandle, "onLibraryLoaded");
	s_gameAPI.onLibraryUnloaded = (GameFunctionPtr)platform::getProcedureAddress(s_gameAPI.libraryHandle, "onLibraryUnloaded");

	YAE_ASSERT(s_gameAPI.gameInit);
	YAE_ASSERT(s_gameAPI.gameUpdate);
	YAE_ASSERT(s_gameAPI.gameShutdown);

	s_gameAPI.onLibraryLoaded();

	YAE_LOG_CAT("game_module", "Loaded Game DLL");
}

void unloadGameAPI()
{
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(s_gameAPI.libraryHandle != nullptr);
	
	s_gameAPI.onLibraryUnloaded();
	
	platform::unloadDynamicLibrary(s_gameAPI.libraryHandle);
	s_gameAPI = {};

	YAE_LOG_CAT("game_module", "Unloaded Game DLL");
}

void watchGameAPI()
{
	YAE_CAPTURE_FUNCTION();

	GameAPI api = s_gameAPI;
	// @TODO: Maybe do a proper file watch at some point. Adding a bit of wait seems to do the trick though
	u64 previousLastWriteTime = s_gameAPI.lastWriteTime;
	u64 lastWriteTime = platform::getFileLastWriteTime(dllSrcPath);
	u64 timeSinceLastWriteTime = platform::getSystemTime() - lastWriteTime;
	if (lastWriteTime > previousLastWriteTime && timeSinceLastWriteTime > 1000000) // 0.1 s
	{
		unloadGameAPI();
		loadGameAPI();

		YAE_LOGF_CAT("game_module", "Hot-Reloaded %s(%lld) > %s(%lld).", dllSrcPath, previousLastWriteTime, dllDstPath, s_gameAPI.lastWriteTime);
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
