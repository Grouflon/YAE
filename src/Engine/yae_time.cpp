#include "yae_time.h"

#include <platform.h>

#include <climits>

namespace { // anonymous namespace

i64 getFrequency()
{
	static i64 s_frequency = yae::platform::getFrequency(); // appparently this never changes over the program
	return s_frequency;
}

} // anonymous namespace

namespace yae {

Time Time::operator+(const Time& _rhs)
{
	return Time(time + _rhs.time);
}


Time Time::operator-(const Time& _rhs)
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

double timeToSeconds64(i64 _time)
{
	return double(timeToMicroSeconds(_time)) / 1000000.0;
}


float timeToSeconds(i64 _time)
{
	return float(timeToMicroSeconds(_time)) / 1000000.f;
}

} // namespace time

} // namespace yae