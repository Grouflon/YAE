#pragma once

#include <types.h>

namespace yae {

struct Time
{
	i64 time;

	Time() {}
	Time(i64 _time) : time(_time) {}

	double asSeconds64() const;
	float asSeconds() const;

	Time operator+(const Time& _rhs);
	Time operator-(const Time& _rhs);
	bool operator==(const Time& _rhs) const;
	bool operator!=(const Time& _rhs) const;
	bool operator< (const Time& _rhs) const;
	bool operator> (const Time& _rhs) const;
	bool operator<=(const Time& _rhs) const;
	bool operator>=(const Time& _rhs) const;
};


struct Clock
{
	Time startTime;

	Clock() : startTime(0) {}

	Time elapsed();
	Time reset();
};


namespace time {

yae::Time now();
yae::Time max();

i64 timeToNanoSeconds(i64 _time);
i64 timeToMicroSeconds(i64 _time);
double timeToSeconds64(i64 _time);
float timeToSeconds(i64 _time);

} // namespace time

} // namespace yae
