#include "time.h"

#include <yae/platform.h>
#include <yae/string.h>

#include <climits>

namespace { // anonymous namespace

i64 getFrequency()
{
	static i64 s_frequency = yae::platform::getFrequency(); // appparently this never changes over the program
	return s_frequency;
}

} // anonymous namespace

namespace yae {

Time Time::operator+(const Time& _rhs) const
{
	return Time(time + _rhs.time);
}


Time Time::operator-(const Time& _rhs) const
{
	return Time(time - _rhs.time);	
}


bool Time::operator==(const Time& _rhs) const
{
	return time == _rhs.time;
}


bool Time::operator!=(const Time& _rhs) const
{
	return time != _rhs.time;
}


bool Time::operator< (const Time& _rhs) const
{
	return time < _rhs.time;
}


bool Time::operator> (const Time& _rhs) const
{
	return time > _rhs.time;
}


bool Time::operator<=(const Time& _rhs) const
{
	return time <= _rhs.time;
}


bool Time::operator>=(const Time& _rhs) const
{
	return time >= _rhs.time;
}


double Time::asMilliSeconds64() const
{
	return time::timeToMilliSeconds64(time);
}


float Time::asMilliSeconds() const
{
	return time::timeToMilliSeconds(time);
}


double Time::asSeconds64() const
{
	return time::timeToSeconds64(time);
}


float Time::asSeconds() const
{
	return time::timeToSeconds(time);
}


Time Clock::elapsed()
{
	Time now = time::now();
	return now - startTime;
}


Time Clock::reset()
{
	Time now = time::now();
	Time elapsed = now - startTime;
	startTime = now;
	return elapsed;
}


namespace time {

Time now()
{
	return yae::Time(platform::getTime());
}

Time max()
{
	return Time(LLONG_MAX);
}

i64 timeToNanoSeconds(i64 _time)
{
	return (_time * 1000000000) / getFrequency();
}

i64 timeToMicroSeconds(i64 _time)
{
	return (_time * 1000000) / getFrequency();
}

double timeToMilliSeconds64(i64 _time)
{
	return double(timeToMicroSeconds(_time)) / 1000.0;
}


float timeToMilliSeconds(i64 _time)
{
	return float(timeToMicroSeconds(_time)) / 1000.f;
}

double timeToSeconds64(i64 _time)
{
	return double(timeToMicroSeconds(_time)) / 1000000.0;
}

float timeToSeconds(i64 _time)
{
	return float(timeToMicroSeconds(_time)) / 1000000.f;
}

void formatTime(Time _time, String& _outString)
{
	const char* units[] = {
		"ns",
		"us",
		"ms",
		"s"
	};

	double time = double(timeToNanoSeconds(_time.time));
	u8 unit = 0;
	while (unit < 3 && time > 1000.0)
	{
		time = time / 1000.0;
		++unit;
	}

	_outString = string::format("%.2f%s", time, units[unit]);
}

} // namespace time

} // namespace yae