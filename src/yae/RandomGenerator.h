#pragma once

#include <yae/types.h>

#include <random>

namespace yae {

struct YAE_API RandomGenerator
{
	RandomGenerator();
	RandomGenerator(u32 _seed);

	std::mt19937 mt;
};

} // namespace yae
