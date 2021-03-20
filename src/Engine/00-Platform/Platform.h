#pragma once

#include <string>

#include <00-Type/IntTypes.h>

namespace yae {

class Platform
{
public:
	static i64 GetTime();
	static i64 GetFrequency();

	static void SetWorkingDirectory(const char* _path);
	static std::string GetWorkingDirectory();
	static bool DuplicateFile(const char* _srcPath, const char* _dstPath);
	static u64 GetFileLastWriteTime(const char* _path);

	static void* LoadDynamicLibrary(const char* _path);
	static void UnloadDynamicLibrary(void* _libraryHandle);
	static void* GetProcedureAddress(void* _libraryHandle, const char* _procedureName);
};

}