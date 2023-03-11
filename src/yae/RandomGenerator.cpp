#include "RandomGenerator.h"

namespace yae {

RandomGenerator::RandomGenerator()
{
	std::random_device rd;
	mt = std::mt19937(rd());
}

RandomGenerator::RandomGenerator(u32 _seed)
{
	mt = std::mt19937(_seed);
}

} // namespace yae
