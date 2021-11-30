#include <yae/platform.h>

#include <yae/callstack.h>
#include <yae/memory.h>

#include <windows.h>
#include <atlstr.h>
#include <dbghelp.h>

namespace yae {

namespace platform {

i64 getCycles()
{
	LARGE_INTEGER largeInteger;
	QueryPerformanceCounter(&largeInteger);
	return largeInteger.QuadPart;
}


i64 getFrequency()
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	return frequency.QuadPart;
}


u64 getSystemTime()
{
	FILETIME filetime = {};
	GetSystemTimeAsFileTime(&filetime);
	return u64(filetime.dwLowDateTime) | u64(filetime.dwHighDateTime) >> 32;
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


bool duplicateFile(const char* _srcPath, const char* _dstPath)
{	
	if (!CopyFileA(_srcPath, _dstPath, FALSE))
	{
		LPVOID lpMsgBuf;
		DWORD dw = GetLastError();

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);

		LPCTSTR msg = (LPCTSTR)lpMsgBuf;
		YAE_ERRORF("Failed to copy %s -> %s: %s", _srcPath, _dstPath, CStringA(msg));
		return false;
	}
	return true;
}


bool doesPathExists(const char* _path)
{
	DWORD fileAttributes = GetFileAttributesA(_path);
	if(INVALID_FILE_ATTRIBUTES == fileAttributes && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
	    return false;
	}
	return true;
}


u64 getFileLastWriteTime(const char* _path)
{
	WIN32_FIND_DATAA fileData;
	FILETIME filetime = {};
	HANDLE handle = FindFirstFileA(_path, &fileData);
	if (handle)
	{
		filetime = fileData.ftLastWriteTime;
		FindClose(handle);
		return u64(filetime.dwLowDateTime) | u64(filetime.dwHighDateTime) >> 32;
	}
	return 0;
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

	IMAGEHLP_SYMBOL64* symbol = (IMAGEHLP_SYMBOL64*)scratchAllocator().allocate(sizeof(IMAGEHLP_SYMBOL64) + 256 * sizeof(TCHAR));
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

    scratchAllocator().deallocate(symbol);
    return frameCount;
}

} // namespace platform

} // namespace yae
