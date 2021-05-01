#pragma once

#include <types.h>

namespace yae {

struct StackFrame
{
	void* instructionPointer;
	void* stackPointer;
	void* basePointer;
	char name[256];
};

YAELIB_API u16 captureCallstack(StackFrame* _outFrames, u16 _maxFrameCount);
YAELIB_API void printCallstack(StackFrame* _frames, u16 _frameCount);

} // namespace yae

#define PRINT_CALLSTACK(_maxFrameCount) { yae::StackFrame frames[_maxFrameCount]; u16 frameCount = yae::captureCallstack(frames, _maxFrameCount); yae::printCallstack(frames, frameCount); }
