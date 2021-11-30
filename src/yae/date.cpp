#include "date.h"

#include <ctime>

namespace yae {

Date Date::operator+(const Date& _rhs) const
{
	return Date(this->time + _rhs.time);
}


Date Date::operator-(const Date& _rhs) const
{
	return Date(this->time - _rhs.time);
}


bool Date::operator==(const Date& _rhs) const
{
	return this->time == _rhs.time;
}


bool Date::operator!=(const Date& _rhs) const
{
	return this->time != _rhs.time;
}


bool Date::operator< (const Date& _rhs) const
{
	return this->time < _rhs.time;
}


bool Date::operator> (const Date& _rhs) const
{
	return this->time > _rhs.time;
}


bool Date::operator<=(const Date& _rhs) const
{
	return this->time <= _rhs.time;
}


bool Date::operator>=(const Date& _rhs) const
{
	return this->time >= _rhs.time;
}



namespace date {

yae::Date now()
{
	return Date(i64(std::time(nullptr)));
}

} // namespace date

} // namespace yae
