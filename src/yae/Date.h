#pragma once

#include <yae/types.h>

namespace yae {

struct YAE_API Date
{
	i64 time;

	Date() {}
	Date(i64 _time) : time(_time) {}

	Date operator+(const Date& _rhs) const;
	Date operator-(const Date& _rhs) const;
	bool operator==(const Date& _rhs) const;
	bool operator!=(const Date& _rhs) const;
	bool operator< (const Date& _rhs) const;
	bool operator> (const Date& _rhs) const;
	bool operator<=(const Date& _rhs) const;
	bool operator>=(const Date& _rhs) const;
};

namespace date {

YAE_API yae::Date now();

} // namespace date

} // namespace yae