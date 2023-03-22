#pragma once

#include <yae/types.h>
#include <yae/containers/HashMap.h>
#include <yae/time.h>

#include <yae/program.h>

namespace yae {

class Allocator;

class YAE_API Profiler
{
public:
	Profiler(Allocator* _allocator);
	~Profiler();

	void pushEvent(const char* _name);
	void popEvent(const char* _name);

	void startCapture(const char* _captureName);
	void stopCapture(const char* _captureName);
	void dropCapture(const char* _captureName);

	void dumpCapture(const char* _captureName, String& _outString) const;

	void update();

// private:
	
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

} // namespace yae
