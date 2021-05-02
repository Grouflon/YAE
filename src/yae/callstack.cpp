#include "callstack.h"

#include <cstdio>

namespace yae {

void printCallstack(StackFrame* _frames, u16 _frameCount)
{
	for (u16 i = 0; i < _frameCount; ++i)
	{
		printf("%02d -> %s (ip:%p)\n", i, _frames[i].name, _frames[i].instructionPointer);
	}
}

} // namespace yae
