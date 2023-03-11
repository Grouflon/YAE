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
	OutputColor_Green,
};

namespace platform {

// Logging
YAE_API void setOutputColor(OutputColor _color);	

// Time
YAE_API i64 getCurrentTime();
YAE_API i64 timeToNanoSeconds(i64 _platformTime);

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

// Window
YAE_API void* findConsoleWindowHandle();
YAE_API void getWindowSize(void* _windowHandle, i32* _outWidth, i32* _outHeight);
YAE_API void getWindowPosition(void* _windowHandle, i32* _outX, i32* _outY);
YAE_API void setWindowSize(void* _windowHandle, i32 _width, i32 _height);
YAE_API void setWindowPosition(void* _windowHandle, i32 _x, i32 _y);

} // namespace platform

} // namespace yae
