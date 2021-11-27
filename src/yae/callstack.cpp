#include "callstack.h"

#include <yae/platform.h>

#include <cstdio>

namespace yae {
namespace callstack {
u16 capture(StackFrame* _outFrames, u16 _maxFrameCount)
{
	return yae::platform::captureCallstack(_outFrames, _maxFrameCount);
}


void print(StackFrame* _frames, u16 _frameCount)
{
	for (u16 i = 0; i < _frameCount; ++i)
	{
		printf("%02d -> %s (ip:%p)\n", i, _frames[i].name, _frames[i].instructionPointer);
	}
}

} // namespace callstack
} // namespace yae
