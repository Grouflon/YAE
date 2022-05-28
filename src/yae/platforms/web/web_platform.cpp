#include <yae/platform.h>

#include <emscripten.h>

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
	return nullptr;
}


void unloadDynamicLibrary(void* _libraryHandle)
{
}


void* getProcedureAddress(void* _libraryHandle, const char* _procedureName)
{
	return nullptr;
}


u16 captureCallstack(StackFrame* _outFrames, u16 _maxFrameCount)
{
	return 0u;
}

} // namespace platform

} // namespace yae
