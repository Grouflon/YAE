#include <yae/platform.h>

namespace yae {

namespace platform {

void setOutputColor(OutputColor _color)
{
}

i64 getCycles()
{
	return 1;
}


i64 getFrequency()
{
	return 1;
}


u64 getSystemTime()
{
	return 0u;
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
