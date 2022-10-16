#pragma once

#include <yae/types.h>

namespace yae {

struct StackFrame;

enum OutputColor
{
	OutputColor_Default,
	OutputColor_Grey,
	OutputColor_Yellow,
	OutputColor_Red,
};

namespace platform {

// Logging
YAE_API void setOutputColor(OutputColor _color);	

// Time
YAE_API i64 getCurrentTime();
YAE_API i64 timeToNanoSeconds(i64 _platformTime);
//YAE_API i64 getCycles();
//YAE_API i64 getFrequency();
//YAE_API u64 getSystemTime();

// File system
YAE_API void setWorkingDirectory(const char* _path);
YAE_API String getWorkingDirectory();
YAE_API String getAbsolutePath(const char* _path);

// DLLs
YAE_API void* loadDynamicLibrary(const char* _path);
YAE_API void unloadDynamicLibrary(void* _libraryHandle);
YAE_API void* getProcedureAddress(void* _libraryHandle, const char* _procedureName);

// Callstack
YAE_API u16 captureCallstack(StackFrame* _outFrames, u16 _maxFrameCount);

} // namespace platform

} // namespace yae
