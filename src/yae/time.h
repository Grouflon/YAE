#pragma once

#include <yae/types.h>

namespace yae {

struct YAE_API Time
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

struct YAE_API Clock
{
	Time startTime;

	Clock() : startTime(0) {}

	Time elapsed();
	Time reset();
};


namespace time {

YAE_API yae::Time now();
YAE_API yae::Time max();

YAE_API i64 timeToNanoSeconds(Time _time);
YAE_API i64 timeToMicroSeconds(Time _time);
YAE_API double timeToMilliSeconds64(Time _time);
YAE_API float timeToMilliSeconds(Time _time);
YAE_API double timeToSeconds64(Time _time);
YAE_API float timeToSeconds(Time _time);

void formatTime(Time _time, String& _outString);

} // namespace time

} // namespace yae
