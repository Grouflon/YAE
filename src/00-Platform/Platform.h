#pragma once

#include <00-Type/IntTypes.h>

namespace yae {

class Platform
{
public:
	static i64 Platform::GetTime();
	static i64 Platform::GetFrequency();
};

}