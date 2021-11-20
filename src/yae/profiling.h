#pragma once

#include <yae/types.h>
#include <yae/time.h>
#include <yae/containers.h>
#include <yae/hash.h>
#include <yae/program.h>

#define YAE_PROFILING_ENABLED 1

#if YAE_PROFILING_ENABLED
#define YAE_CAPTURE_START(_captureName) yae::profiler().startCapture(_captureName)
#define YAE_CAPTURE_STOP(_captureName) yae::profiler().stopCapture(_captureName)
#define YAE_CAPTURE_SCOPE(_scopeName) CaptureScope __scope##__LINE__(_scopeName)
#define YAE_CAPTURE_FUNCTION() YAE_CAPTURE_SCOPE(__FUNCTION__)
#else
#define YAE_CAPTURE_START(_captureName)
#define YAE_CAPTURE_STOP(_captureName)
#define YAE_CAPTURE_SCOPE(_scopeName)
#define YAE_CAPTURE_FUNCTION()
#endif

namespace yae {

class Allocator;

class YAELIB_API Profiler
{
public:
	Profiler(Allocator* _allocator);
	~Profiler();

	void pushEvent(const char* _name);
	void popEvent(const char* _name);

	void startCapture(const char* _captureName);
	void stopCapture(const char* _captureName);

	void dumpCapture(const char* _captureName, String& _outString) const;

	void update();

private:

	void _cleanEvents();

	struct Event
	{
		const char* name;
		Time startTime;
		Time stopTime;
	};

	struct Capture
	{
		Capture() {};
		Capture(Allocator* _allocator) : events(_allocator) {}

		const char* name;
		Time startTime;
		Time stopTime;
		DataArray<Event> events;
	};

	Allocator* m_allocator = nullptr;

	DataArray<Event> m_events;
	DataArray<u16> m_eventsStack;

	HashMap<StringHash, Capture> m_runningCaptures;
	HashMap<StringHash, Capture> m_captures;
};

struct CaptureScope
{
	CaptureScope(const char* _name);
	~CaptureScope();

	const char* m_name;
};


} // namespace yae
