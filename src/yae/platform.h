#pragma once

#include <yae/types.h>
#include <yae/string.h>

namespace yae {

namespace platform {

YAELIB_API i64 getTime();
YAELIB_API i64 getFrequency();

YAELIB_API void setWorkingDirectory(const char* _path);
YAELIB_API String getWorkingDirectory();
YAELIB_API bool duplicateFile(const char* _srcPath, const char* _dstPath);
YAELIB_API bool doesPathExists(const char* _path);
YAELIB_API u64 getFileLastWriteTime(const char* _path);
YAELIB_API u64 getSystemTime();

YAELIB_API void* loadDynamicLibrary(const char* _path);
YAELIB_API void unloadDynamicLibrary(void* _libraryHandle);
YAELIB_API void* getProcedureAddress(void* _libraryHandle, const char* _procedureName);

} // namespace platform

} // namespace yae
