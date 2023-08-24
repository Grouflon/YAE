#include "callstack.h"

#include <core/platform.h>

#include <cstdio>

namespace yae {
namespace callstack {
u16 capture(StackFrame* _outFrames, u16 _maxFrameCount)
{
	return yae::platform::captureCallstack(_outFrames, _maxFrameCount);
}


void print(StackFrame* _frames, u16 _frameCount)
{
	//@NOTE: the first index is always the capture function, so we skip it
	for (u16 i = 1; i < _frameCount; ++i)
	{
		printf("%02d -> %s (ip:0x%p)\n", i - 1, _frames[i].name, _frames[i].instructionPointer);
	}
}

} // namespace callstack
} // namespace yae
