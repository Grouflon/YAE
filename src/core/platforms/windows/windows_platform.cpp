#include <core/platform.h>

#include <core/callstack.h>
#include <core/memory.h>
#include <core/filesystem.h>
#include <core/string.h>

#include <windows.h>
#include <psapi.h>
#include <dbghelp.h>
#include <chrono>

#undef ERROR //NOTE: Windows header messes with our log types

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
		case OutputColor_Green: colorAttribute = 2; break;

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
	FreeLibrary((HMODULE)_libraryHandle);
}

void* getProcedureAddress(void* _libraryHandle, const char* _procedureName)
{
	HMODULE module = (HMODULE)_libraryHandle;
	return (void*)GetProcAddress(module, _procedureName);
}

static int s_symbolsLoadedCount = 0;

void loadSymbols()
{
	if (s_symbolsLoadedCount == 0)
	{
		HANDLE hProcess = GetCurrentProcess();
		SymInitialize(hProcess, NULL, TRUE);
	}
	++s_symbolsLoadedCount;
}

void unloadSymbols()
{
	--s_symbolsLoadedCount;
	YAE_ASSERT(s_symbolsLoadedCount >= 0);
	if (s_symbolsLoadedCount == 0)
	{
		HANDLE hProcess = GetCurrentProcess();
		SymCleanup(hProcess);
	}
}

u16 captureCallstack(StackFrame* _outFrames, u16 _maxFrameCount)
{
	loadSymbols();

	HANDLE hProcess = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();
	DWORD64 displacement = 0;

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

    unloadSymbols();
    return frameCount;
}

// NOTE: This does not work yet. The back and forth between name and address is not really understood
String getSymbolNameFromAddress(void* _address)
{
	loadSymbols();

	HANDLE hProcess = GetCurrentProcess();

	DWORD64 dwDisplacement = 0;
	DWORD64 dwAddress = (DWORD64)_address;

	char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
	PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;

	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	pSymbol->MaxNameLen = MAX_SYM_NAME;

	DWORD options = SymGetOptions();
	SymSetOptions(options|SYMOPT_UNDNAME);

	if (SymFromAddr(hProcess, dwAddress, &dwDisplacement, pSymbol))
	{
		//return pSymbol->Name;
		// try the simple thing first

		size_t nameSize = strlen(pSymbol->Name);
		size_t symbolNameStart = 0;
		size_t symbolNameEnd = nameSize;

		for (size_t i = 0; i < nameSize; ++i)
		{
			if (pSymbol->Name[i] == '?')
			{
				symbolNameStart = i;
				break;
			}
		}
		for (size_t i = 0; i < nameSize; ++i)
		{
			if (pSymbol->Name[nameSize-1-i] == ')')
			{
				symbolNameEnd = nameSize-1-i;
				break;
			}
		}

		String256 undecoratedSymbol = pSymbol->Name;
		undecoratedSymbol = string::slice(undecoratedSymbol, symbolNameStart, symbolNameEnd - symbolNameStart);

		char output[256];
		if (UnDecorateSymbolName(undecoratedSymbol.c_str(), output, sizeof(output), UNDNAME_COMPLETE))
		{
		    // UnDecorateSymbolName returned success
			YAE_LOGF("%s -> %s -> %p", pSymbol->Name, output, pSymbol->Address);
			unloadSymbols();
			return output;
		}
		else
		{
		    // UnDecorateSymbolName failed
		    DWORD error = GetLastError();
		    YAE_ERRORF("UnDecorateSymbolName returned error %d\n", error);
		}
	    // SymFromAddr returned success
	}
	else
	{
	    // SymFromAddr failed
	    DWORD error = GetLastError();
	    YAE_ERRORF("SymFromAddr returned error : %d\n", error);
	}
	unloadSymbols();
	return "";
}

// NOTE: This does not work yet. The back and forth between name and address is not really understood
void* getAddressFromSymbolName(const char* _symbolName)
{
	loadSymbols();
	HANDLE hProcess = GetCurrentProcess();

	char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
	PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;

	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	pSymbol->MaxNameLen = MAX_SYM_NAME;

	if (SymFromName(hProcess, _symbolName, pSymbol))
	{
		SymCleanup(hProcess);
	    return (void*)pSymbol->Address;
	}
	else
	{
	    // SymFromName failed
	    DWORD error = GetLastError();
	    YAE_ERRORF("SymFromName returned error : %d\n", error);
	}
	
	unloadSymbols();
	return nullptr;
}

void debugSymbols(void* _address, const char* _addressName)
{
	loadSymbols();
	HANDLE hProcess = GetCurrentProcess();

	DWORD64  dwDisplacement = 0;
	DWORD64  dwAddress = (DWORD64)_address;

	char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
	PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;

	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	pSymbol->MaxNameLen = MAX_SYM_NAME;

	if (SymFromAddr(hProcess, dwAddress, &dwDisplacement, pSymbol))
	{
		char output[256];
		if (UnDecorateSymbolName(pSymbol->Name, output, sizeof(output), UNDNAME_32_BIT_DECODE | UNDNAME_NAME_ONLY))
		{
		    // UnDecorateSymbolName returned success
			YAE_LOGF("%s -> %s -> %p", pSymbol->Name, output, pSymbol->Address);
		}
		else
		{
		    // UnDecorateSymbolName failed
		    DWORD error = GetLastError();
		    YAE_LOGF("UnDecorateSymbolName returned error %d\n", error);
		}
	    // SymFromAddr returned success
	}
	else
	{
	    // SymFromAddr failed
	    DWORD error = GetLastError();
	    YAE_LOGF("SymFromAddr returned error : %d\n", error);
	}

	unloadSymbols();
	return;

	// EnumerateLoadedModulesEx(hProcess, [](PCSTR ModuleName, ULONG64 ModuleBase, ULONG ModuleSize, PVOID UserContext)
	// {
	// 	YAE_LOGF("%s(%p)", ModuleName, ModuleBase);
	// 	return 1;
	// }, nullptr);

// 	typedef struct _SYMBOL_INFOW {
//     ULONG       SizeOfStruct;
//     ULONG       TypeIndex;        // Type Index of symbol
//     ULONG64     Reserved[2];
//     ULONG       Index;
//     ULONG       Size;
//     ULONG64     ModBase;          // Base Address of module comtaining this symbol
//     ULONG       Flags;
//     ULONG64     Value;            // Value of symbol, ValuePresent should be 1
//     ULONG64     Address;          // Address of symbol including base address of module
//     ULONG       Register;         // register holding value or pointer to value
//     ULONG       Scope;            // scope of the symbol
//     ULONG       Tag;              // pdb classification
//     ULONG       NameLen;          // Actual length of name
//     ULONG       MaxNameLen;
//     WCHAR       Name[1];          // Name of symbol
// } SYMBOL_INFOW, *PSYMBOL_INFOW;

	struct Data
	{
		const char* name;
		void* addr;
	} data = {_addressName, _address};

	SymEnumSymbols(hProcess, 0, "*!*", [](PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext)
	{
		Data* dataPtr = (Data*)UserContext;
		String name = pSymInfo->Name;
		if (string::find(name, dataPtr->name) != string::INVALID_POS)
		{
			char* offset = (char*)pSymInfo->Address - (ULONG64)dataPtr->addr;
			YAE_LOGF("%s\t-> sym:%p, add:%p, off:%p", dataPtr->name, pSymInfo->Address, dataPtr->addr, offset);
		}

		//YAE_ASSERT(pSymInfo->Address != (ULONG64)UserContext);
		//YAE_LOGF("%s(%p)", pSymInfo->Name, pSymInfo->Address);
		return 1;
	}, &data);
}

void* findConsoleWindowHandle()
{
	char titleBuffer[1024];
	GetConsoleTitle(titleBuffer, countof(titleBuffer));

	HWND hwndFound = FindWindow(nullptr, titleBuffer);
	return hwndFound;
}

void getWindowSize(void* _windowHandle, i32* _outWidth, i32* _outHeight)
{
	YAE_ASSERT(_windowHandle != nullptr);
	WINDOWINFO windowInfo;
	YAE_VERIFY(GetWindowInfo((HWND)_windowHandle, &windowInfo));
	if (_outWidth) *_outWidth = windowInfo.rcWindow.right - windowInfo.rcWindow.left;
	if (_outHeight) *_outHeight = windowInfo.rcWindow.bottom - windowInfo.rcWindow.top;
}

void getWindowPosition(void* _windowHandle, i32* _outX, i32* _outY)
{
	YAE_ASSERT(_windowHandle != nullptr);
	WINDOWINFO windowInfo;
	YAE_VERIFY(GetWindowInfo((HWND)_windowHandle, &windowInfo));
	if (_outX) *_outX = windowInfo.rcWindow.left;
	if (_outY) *_outY = windowInfo.rcWindow.top;
}

void setWindowSize(void* _windowHandle, i32 _width, i32 _height)
{
	YAE_ASSERT(_windowHandle != nullptr);
	u32 flags = SWP_NOREPOSITION | SWP_NOZORDER | SWP_NOACTIVATE;
	YAE_VERIFY(SetWindowPos((HWND)_windowHandle, 0, 0, 0, _width, _height, flags));
}

void setWindowPosition(void* _windowHandle, i32 _x, i32 _y)
{
	YAE_ASSERT(_windowHandle != nullptr);
	u32 flags = SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE;
	YAE_VERIFY(SetWindowPos((HWND)_windowHandle, 0, _x, _y, 0, 0, flags));
}

} // namespace platform

} // namespace yae
