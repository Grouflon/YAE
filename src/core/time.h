#pragma once

#include <core/types.h>

namespace yae {

struct CORE_API Time
{
	i64 time;

	Time() {}
	Time(i64 _time) : time(_time) {}

	double asSeconds64() const;
	float asSeconds() const;
	double asMilliSeconds64() const;
	float asMilliSeconds() const;

	Time operator+(const Time& _rhs) const;
	Time operator-(const Time& _rhs) const;
	bool operator==(const Time& _rhs) const;
	bool operator!=(const Time& _rhs) const;
	bool operator< (const Time& _rhs) const;
	bool operator> (const Time& _rhs) const;
	bool operator<=(const Time& _rhs) const;
	bool operator>=(const Time& _rhs) const;
};

struct CORE_API Clock
{
	Time startTime;

	Clock() : startTime(0) {}

	Time elapsed();
	Time reset();
};


namespace time {

CORE_API yae::Time now();
CORE_API yae::Time max();

CORE_API i64 timeToNanoSeconds(Time _time);
CORE_API i64 timeToMicroSeconds(Time _time);
CORE_API double timeToMilliSeconds64(Time _time);
CORE_API float timeToMilliSeconds(Time _time);
CORE_API double timeToSeconds64(Time _time);
CORE_API float timeToSeconds(Time _time);

void formatTime(Time _time, String& _outString);

} // namespace time

} // namespace yae
