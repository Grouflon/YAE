#pragma once

#include <yae/types.h>

namespace yae {

struct StackFrame
{
	void* instructionPointer;
	void* stackPointer;
	void* basePointer;
	char name[256];
};

namespace callstack {

YAE_API u16 capture(StackFrame* _outFrames, u16 _maxFrameCount);
YAE_API void print(StackFrame* _frames, u16 _frameCount);

} // namespace callstack

} // namespace yae

#define PRINT_CALLSTACK(_maxFrameCount) { yae::StackFrame frames[_maxFrameCount]; u16 frameCount = yae::callstack::capture(frames, _maxFrameCount); yae::callstack::print(frames, frameCount); }
