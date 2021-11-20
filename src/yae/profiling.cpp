#include "profiling.h"

#include <yae/memory.h>

namespace yae {

Profiler::Profiler(Allocator* _allocator)
	: m_allocator(_allocator)
	, m_events(_allocator)
	, m_eventsStack(_allocator)
	, m_runningCaptures(_allocator)
	, m_captures(_allocator)
{

}


Profiler::~Profiler()
{
	
}


void Profiler::pushEvent(const char* _name)
{
	Event event;
	event.name = _name;
	event.startTime = time::now();
	event.stopTime = Time(0);
	m_eventsStack.push_back(m_events.size());
	m_events.push_back(event);
}


void Profiler::popEvent(const char* _name)
{
	u16 eventIndex = m_eventsStack.back();
	Event& event = m_events[eventIndex];
	YAE_ASSERT(strcmp(event.name, _name) == 0);
	event.stopTime = time::now();
	m_eventsStack.pop_back();
}


void Profiler::startCapture(const char* _captureName)
{
	Capture& capture = m_runningCaptures.set(StringHash(_captureName), Capture(m_allocator));
	capture.name = _captureName;
	capture.startTime = time::now();
	capture.stopTime = Time(0);
}


void Profiler::stopCapture(const char* _captureName)
{
	StringHash nameHash(_captureName);
	Capture* runningCapturePtr = m_runningCaptures.get(nameHash);
	YAE_ASSERT(runningCapturePtr != nullptr);
	Capture& capture = m_captures.set(nameHash, *runningCapturePtr);
	m_runningCaptures.remove(nameHash);

	capture.stopTime = time::now();
	for (const Event& e : m_events)
	{
		if (e.startTime > capture.stopTime)
			break;

		if (
			(e.startTime >= capture.startTime && e.stopTime <= capture.stopTime) ||
			(e.startTime < capture.startTime && e.stopTime > capture.startTime) ||
			(e.startTime < capture.stopTime && e.stopTime > capture.stopTime)
		)
		{
			capture.events.push_back(e);
		}
	}
}


void Profiler::dumpCapture(const char* _captureName, String& _outString) const
{
	StringHash nameHash(_captureName);
	const Capture* capturePtr = m_captures.get(nameHash);

	_outString += string::format("-- %s: ", _captureName);

	if (!capturePtr)
	{
		_outString = "no capture";
		return;
	}

	Time captureTime = capturePtr->stopTime - capturePtr->startTime;

	String32 timeString;
	time::formatTime(captureTime, timeString);
	_outString += string::format("%s\n", timeString.c_str());

	String64 tabs;
	DataArray<u32> stack(&scratchAllocator());
	for (u32 i = 0; i < capturePtr->events.size(); ++i)
	{
		const Event& e = capturePtr->events[i];

		for (i32 j = stack.size() - 1; j >= 0; --j)
		{
			if (e.startTime >= capturePtr->events[stack[j]].stopTime)
			{
				stack.pop_back();
			}
		}

		stack.push_back(i);

		tabs.clear();
		for (u32 j = 1; j < stack.size(); ++j)
		{
			tabs += "  ";
		}

		Time eventTime = e.stopTime - e.startTime;

		timeString.clear();
		time::formatTime(eventTime, timeString);
		_outString += string::format("%s%s: %s\n", tabs.c_str(), e.name, timeString.c_str());
	}
}


void Profiler::update()
{
	YAE_CAPTURE_FUNCTION();

	_cleanEvents();
	
}


void Profiler::_cleanEvents()
{
	u32 removedEvents = 0;
	if (m_runningCaptures.empty())
	{
		// if no capture is running, we can ditch everything
		removedEvents = m_events.size();
		m_events.clear();
	}
	else
	{
		// if not, let's just remove all the events that are older than the oldest running capture
		Time minTime = time::max();
		for (const auto& pair : m_runningCaptures)
		{
			if (pair.value.startTime < minTime)
			{
				minTime = pair.value.startTime;
			}
		}

		u32 lastOutdatedEventIndex = -1;
		for (u32 i = 0; i < m_events.size(); ++i)
		{
			if (m_events[i].stopTime == Time(0))
			{
				break;
			}

			if (m_events[i].stopTime < minTime)
			{
				lastOutdatedEventIndex = i;
			}
			else
			{
				break;
			}
		}

		if (lastOutdatedEventIndex != -1)
		{
			removedEvents = lastOutdatedEventIndex + 1;
			m_events.erase(0, removedEvents);
		}	
	}

	// update indices inside the stack
	for (u32 i = 0; i < m_eventsStack.size(); ++i)
	{
		m_eventsStack[i] -= removedEvents;
	}
}


CaptureScope::CaptureScope(const char* _name)
	: m_name(_name)
{
	profiler().pushEvent(_name);
}


CaptureScope::~CaptureScope()
{
	profiler().popEvent(m_name);
}


} // namespace yae
