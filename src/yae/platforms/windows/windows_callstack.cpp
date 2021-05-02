#include <yae/callstack.h>

#include <windows.h>
#include <dbghelp.h>

namespace yae {

u16 captureCallstack(StackFrame* _outFrames, u16 _maxFrameCount)
{
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();
	DWORD64 displacement = 0;

	SymInitialize(hProcess, NULL, TRUE);

    STACKFRAME64 stackFrame = {};
    CONTEXT context;
    RtlCaptureContext(&context);

    stackFrame.AddrPC.Offset       = context.Rip;
	stackFrame.AddrPC.Mode         = AddrModeFlat;
	stackFrame.AddrStack.Offset    = context.Rsp;
	stackFrame.AddrStack.Mode      = AddrModeFlat;
	stackFrame.AddrFrame.Offset    = context.Rbp;
	stackFrame.AddrFrame.Mode      = AddrModeFlat;

	IMAGEHLP_SYMBOL64* symbol = (IMAGEHLP_SYMBOL64*)malloc(sizeof(IMAGEHLP_SYMBOL64) + 256 * sizeof(TCHAR));
	symbol->SizeOfStruct  = sizeof( IMAGEHLP_SYMBOL64 );
    symbol->MaxNameLength = 255;

    u16 frameCount = 0;

    for (u16 frame = 0; frame <= _maxFrameCount; ++frame)
    {
        bool ret = StackWalk64(
            IMAGE_FILE_MACHINE_AMD64,
            hProcess,
            hThread,
            &stackFrame,
            &context,
            NULL,
            SymFunctionTableAccess64,
            SymGetModuleBase64,
            NULL
        );

        if (frame == 0)
        	continue;

        if (!ret)
            break;

        _outFrames[frame - 1].instructionPointer = (void*)(stackFrame.AddrPC.Offset);
        _outFrames[frame - 1].stackPointer = (void*)(stackFrame.AddrStack.Offset);
        _outFrames[frame - 1].basePointer = (void*)(stackFrame.AddrFrame.Offset);

        SymGetSymFromAddr64( hProcess, ( ULONG64 )stackFrame.AddrPC.Offset, &displacement, symbol );
        UnDecorateSymbolName( symbol->Name, ( PSTR )_outFrames[frame - 1].name, 256, UNDNAME_COMPLETE );

        ++frameCount;
    }

    free(symbol);
    return frameCount;
}

} // namespace yae
