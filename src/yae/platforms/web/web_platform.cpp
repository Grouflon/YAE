#include <yae/platform.h>

#include <emscripten.h>
#include <dlfcn.h>

namespace yae {

namespace platform {

void setOutputColor(OutputColor _color)
{
}


i64 getCurrentTime()
{
	double time = emscripten_get_now(); // in ms
	return i64(time * 1000 * 1000); // in ns
}


i64 timeToNanoSeconds(i64 _platformTime)
{
	return _platformTime;
}


void setWorkingDirectory(const char* _path)
{
}


String getWorkingDirectory()
{
	return "";
}


String getAbsolutePath(const char* _path)
{
	return "";
}


void* loadDynamicLibrary(const char* _path)
{
    return dlopen(_path, RTLD_NOW);
}


void unloadDynamicLibrary(void* _libraryHandle)
{
	dlclose(_libraryHandle);
}


void* getProcedureAddress(void* _libraryHandle, const char* _procedureName)
{
	return dlsym(_libraryHandle, _procedureName);
}


u16 captureCallstack(StackFrame* _outFrames, u16 _maxFrameCount)
{
	return 0u;
}

void* findConsoleWindowHandle()
{
	return nullptr;
}

void getWindowSize(void* _windowHandle, i32* _outWidth, i32* _outHeight)
{
}

void getWindowPosition(void* _windowHandle, i32* _outX, i32* _outY)
{
}

void setWindowSize(void* _windowHandle, i32 _width, i32 _height)
{
}

void setWindowPosition(void* _windowHandle, i32 _x, i32 _y)
{
}

} // namespace platform

} // namespace yae
