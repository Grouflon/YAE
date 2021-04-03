#pragma once

#include <types.h>
#include <yae_string.h>

namespace yae {

namespace platform {

i64 getTime();
i64 getFrequency();

void setWorkingDirectory(const char* _path);
String getWorkingDirectory();
bool duplicateFile(const char* _srcPath, const char* _dstPath);
u64 getFileLastWriteTime(const char* _path);

void* loadDynamicLibrary(const char* _path);
void unloadDynamicLibrary(void* _libraryHandle);
void* getProcedureAddress(void* _libraryHandle, const char* _procedureName);

} // namespace platform

} // namespace yae