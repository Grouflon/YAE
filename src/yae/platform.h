#pragma once

#include <yae/types.h>
#include <yae/string.h>

namespace yae {

struct StackFrame;

namespace platform {

// Time
YAELIB_API i64 getTime();
YAELIB_API i64 getFrequency();
YAELIB_API u64 getSystemTime();// This needs to be in the same reference as getFileLastWriteTime

// File system
YAELIB_API void setWorkingDirectory(const char* _path);
YAELIB_API String getWorkingDirectory();
YAELIB_API bool duplicateFile(const char* _srcPath, const char* _dstPath);
YAELIB_API bool doesPathExists(const char* _path);
YAELIB_API u64 getFileLastWriteTime(const char* _path); // This needs to be in the same reference as getSystemTime
YAELIB_API String getAbsolutePath(const char* _path);

// DLLs
YAELIB_API void* loadDynamicLibrary(const char* _path);
YAELIB_API void unloadDynamicLibrary(void* _libraryHandle);
YAELIB_API void* getProcedureAddress(void* _libraryHandle, const char* _procedureName);

// Callstack
YAELIB_API u16 captureCallstack(StackFrame* _outFrames, u16 _maxFrameCount);

} // namespace platform

} // namespace yae
