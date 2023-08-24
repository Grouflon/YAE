#pragma once

#include <core/types.h>

namespace yae {

struct StackFrame;

enum OutputColor
{
	OutputColor_Default,
	OutputColor_Grey,
	OutputColor_Yellow,
	OutputColor_Red,
	OutputColor_Green,
};

namespace platform {

// Logging
CORE_API void setOutputColor(OutputColor _color);	

// Time
CORE_API i64 getCurrentTime();
CORE_API i64 timeToNanoSeconds(i64 _platformTime);

// File system
CORE_API void setWorkingDirectory(const char* _path);
CORE_API String getWorkingDirectory();
CORE_API String getAbsolutePath(const char* _path);

// DLLs
CORE_API void* loadDynamicLibrary(const char* _path);
CORE_API void unloadDynamicLibrary(void* _libraryHandle);
CORE_API void* getProcedureAddress(void* _libraryHandle, const char* _procedureName);

// Callstack
CORE_API u16 captureCallstack(StackFrame* _outFrames, u16 _maxFrameCount);

// Window
CORE_API void* findConsoleWindowHandle();
CORE_API void getWindowSize(void* _windowHandle, i32* _outWidth, i32* _outHeight);
CORE_API void getWindowPosition(void* _windowHandle, i32* _outX, i32* _outY);
CORE_API void setWindowSize(void* _windowHandle, i32 _width, i32 _height);
CORE_API void setWindowPosition(void* _windowHandle, i32 _x, i32 _y);

} // namespace platform

} // namespace yae
