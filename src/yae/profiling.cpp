#include <yae/types.h>
#include "profiling.h"

#include <yae/profiler.h>

namespace yae {
namespace profiling {

CaptureScope::CaptureScope(const char* _name)
	: m_name(_name)
{
	profiler().pushEvent(_name);
}


CaptureScope::~CaptureScope()
{
	profiler().popEvent(m_name);
}

void startCapture(const char* _captureName)
{
	profiler().startCapture(_captureName);
}


void stopCapture(const char* _captureName)
{
	profiler().stopCapture(_captureName);
}

} // namespace profiling
} // namespace yae
