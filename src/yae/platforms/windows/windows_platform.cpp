#include <yae/platform.h>
#include <yae/log.h>

#include <windows.h>
#include <atlstr.h>

namespace yae {

namespace platform {

i64 getTime()
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



u64 getSystemTime()
{
	FILETIME filetime = {};
	GetSystemTimeAsFileTime(&filetime);
	return u64(filetime.dwLowDateTime) | u64(filetime.dwHighDateTime) >> 32;
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

} // namespace platform

} // namespace yae
