#include "Platform.h"

#include <windows.h>

#include <00-Macro/Assert.h>
#include <00-String/StringTools.h>

namespace yae {

i64 Platform::GetTime()
{
	LARGE_INTEGER largeInteger;
	QueryPerformanceCounter(&largeInteger);
	return largeInteger.QuadPart;
}

i64 Platform::GetFrequency()
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	return frequency.QuadPart;
}

void Platform::SetWorkingDirectory(const char* _path)
{
	bool result = SetCurrentDirectoryA(_path);
	YAE_ASSERT(result);
}

std::string Platform::GetWorkingDirectory()
{
	std::string result;
	DWORD bufferSize = GetCurrentDirectoryA(0, NULL);
	result.resize(bufferSize);
	GetCurrentDirectoryA(bufferSize, (char*)result.data());
	return result;
}

bool Platform::DuplicateFile(const char* _srcPath, const char* _dstPath)
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
		int a = 0;
	}
	return true;
}

u64 Platform::GetFileLastWriteTime(const char* _path)
{
	WIN32_FIND_DATAA fileData;
	FILETIME filetime = {};
	HANDLE handle = FindFirstFileA(_path, &fileData);
	if (handle)
	{
		filetime = fileData.ftLastWriteTime;
		FindClose(handle);
	}
	return u64(filetime.dwLowDateTime) | u64(filetime.dwHighDateTime) >> 32;
}

void* Platform::LoadDynamicLibrary(const char* _path)
{
	HMODULE module = LoadLibraryA(_path);
	return module;
}

void Platform::UnloadDynamicLibrary(void* _libraryHandle)
{
	FreeLibrary((HMODULE)_libraryHandle);
}

void* Platform::GetProcedureAddress(void* _libraryHandle, const char* _procedureName)
{
	HMODULE module = (HMODULE)_libraryHandle;
	return (void*)GetProcAddress(module, _procedureName);
}

}
