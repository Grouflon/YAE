#pragma once

#include <core/types.h>

namespace yae {

struct StackFrame
{
	void* instructionPointer;
	void* stackPointer;
	void* basePointer;
	char name[256];
};

namespace callstack {

CORE_API u16 capture(StackFrame* _outFrames, u16 _maxFrameCount);
CORE_API void print(StackFrame* _frames, u16 _frameCount);

} // namespace callstack

} // namespace yae

#define PRINT_CALLSTACK(_maxFrameCount) { yae::StackFrame frames[_maxFrameCount]; u16 frameCount = yae::callstack::capture(frames, _maxFrameCount); yae::callstack::print(frames, frameCount); }
