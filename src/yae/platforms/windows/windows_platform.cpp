#include <yae/platform.h>

#include <yae/callstack.h>
#include <yae/memory.h>
#include <yae/filesystem.h>

#include <windows.h>
#include <dbghelp.h>
#include <chrono>

namespace yae {

namespace platform {

void setOutputColor(OutputColor _color)
{
	// @NOTE: see this thread for help on colors: https://stackoverflow.com/questions/8765938/colorful-text-using-printf-in-c
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	int colorAttribute = 0;
	switch(_color)
	{
		case OutputColor_Grey: colorAttribute = 8; break;
		case OutputColor_Yellow: colorAttribute = 6; break;
		case OutputColor_Red: colorAttribute = 4; break;

		case OutputColor_Default:
		default: colorAttribute = 7; break;
	}
	SetConsoleTextAttribute(hConsole, colorAttribute);
}


i64 getCurrentTime()
{
	auto stdTime = std::chrono::high_resolution_clock::now().time_since_epoch();
	return std::chrono::duration_cast<std::chrono::nanoseconds>(stdTime).count();
}


i64 timeToNanoSeconds(i64 _platformTime)
{
	return _platformTime;
}

void setWorkingDirectory(const char* _path)
{
	bool result = SetCurrentDirectoryA(_path);
	YAE_ASSERT(result);
}


String getWorkingDirectory()
{
	String result;
	DWORD bufferSize = GetCurrentDirectoryA(0, NULL);
	result.resize(bufferSize);
	GetCurrentDirectoryA(bufferSize, (char*)result.data());
	return result;
}


String getAbsolutePath(const char* _path)
{
	static const int BUFFER_SIZE = MAX_PATH;
	char buffer[BUFFER_SIZE];
	bool result = GetFullPathNameA(_path, BUFFER_SIZE, buffer, nullptr);
	if (!result)
		return String(_path, &scratchAllocator());

	return String(buffer, &scratchAllocator());
}


void* loadDynamicLibrary(const char* _path)
{
	HMODULE module = LoadLibraryA(_path);
	return module;
}


void unloadDynamicLibrary(void* _libraryHandle)
{
	HMODULE hModule = (HMODULE)_libraryHandle;
	FreeLibrary(hModule);
}


void* getProcedureAddress(void* _libraryHandle, const char* _procedureName)
{
	HMODULE module = (HMODULE)_libraryHandle;
	return (void*)GetProcAddress(module, _procedureName);
}


u16 captureCallstack(StackFrame* _outFrames, u16 _maxFrameCount)
{
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();
	DWORD64 displacement = 0;

	SymInitialize(hProcess, NULL, TRUE);

    STACKFRAME64 stackFrame = {};
    CONTEXT context;
    RtlCaptureContext(&context);

    stackFrame.AddrPC.Offset       = context.Rip;
	stackFrame.AddrPC.Mode         = AddrModeFlat;
	stackFrame.AddrStack.Offset    = context.Rsp;
	stackFrame.AddrStack.Mode      = AddrModeFlat;
	stackFrame.AddrFrame.Offset    = context.Rbp;
	stackFrame.AddrFrame.Mode      = AddrModeFlat;

	IMAGEHLP_SYMBOL64* symbol = (IMAGEHLP_SYMBOL64*)malloc(sizeof(IMAGEHLP_SYMBOL64) + 256 * sizeof(TCHAR));
	symbol->SizeOfStruct  = sizeof( IMAGEHLP_SYMBOL64 );
    symbol->MaxNameLength = 255;

    u16 frameCount = 0;

    for (u16 frame = 0; frame <= _maxFrameCount; ++frame)
    {
        bool ret = StackWalk64(
            IMAGE_FILE_MACHINE_AMD64,
            hProcess,
            hThread,
            &stackFrame,
            &context,
            NULL,
            SymFunctionTableAccess64,
            SymGetModuleBase64,
            NULL
        );

        if (frame == 0)
        	continue;

        if (!ret)
            break;

        _outFrames[frame - 1].instructionPointer = (void*)(stackFrame.AddrPC.Offset);
        _outFrames[frame - 1].stackPointer = (void*)(stackFrame.AddrStack.Offset);
        _outFrames[frame - 1].basePointer = (void*)(stackFrame.AddrFrame.Offset);

        SymGetSymFromAddr64( hProcess, ( ULONG64 )stackFrame.AddrPC.Offset, &displacement, symbol );
        UnDecorateSymbolName( symbol->Name, ( PSTR )_outFrames[frame - 1].name, 256, UNDNAME_COMPLETE );

        ++frameCount;
    }

    free(symbol);
    return frameCount;
}

} // namespace platform

} // namespace yae
