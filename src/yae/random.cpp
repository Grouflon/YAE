#include "random.h"

#include <yae/RandomGenerator.h>

namespace yae {
namespace random {

static RandomGenerator s_globalRandomGenerator;

void setSeed(u32 _seed)
{
	setSeed(s_globalRandomGenerator, _seed);
}

void setSeed(RandomGenerator& _generator, u32 _seed)
{
	_generator.mt = std::mt19937(_seed);
}

float range01()
{
	return range01(s_globalRandomGenerator);
}

float range01(RandomGenerator& _generator)
{
	return range(_generator, 0.f, 1.f);
}

template <> float range(float _min, float _max)
{
	return range(s_globalRandomGenerator, _min, _max);
}

template <> float range(RandomGenerator& _generator, float _min, float _max)
{
	YAE_ASSERT(_min <= _max);
	std::uniform_real_distribution<float> dist(_min, _max);
	return dist(_generator.mt);
}

template <> double range(double _min, double _max)
{
	return range(s_globalRandomGenerator, _min, _max);
}

template <> double range(RandomGenerator& _generator, double _min, double _max)
{
	YAE_ASSERT(_min <= _max);
	std::uniform_real_distribution<double> dist(_min, _max);
	return dist(_generator.mt);
}

template <> i64 range(i64 _min, i64 _max)
{
	return range(s_globalRandomGenerator, _min, _max);
}

template <> i64 range(RandomGenerator& _generator, i64 _min, i64 _max)
{
	YAE_ASSERT(_min <= _max);
	std::uniform_int_distribution<i64> dist(_min, _max);
	return dist(_generator.mt);
}

template <> i32 range(i32 _min, i32 _max)
{
	return range(s_globalRandomGenerator, _min, _max);
}

template <> i32 range(RandomGenerator& _generator, i32 _min, i32 _max)
{
	YAE_ASSERT(_min <= _max);
	std::uniform_int_distribution<i32> dist(_min, _max);
	return dist(_generator.mt);
}

template <> i16 range(i16 _min, i16 _max)
{
	return range(s_globalRandomGenerator, _min, _max);
}

template <> i16 range(RandomGenerator& _generator, i16 _min, i16 _max)
{
	YAE_ASSERT(_min <= _max);
	std::uniform_int_distribution<i16> dist(_min, _max);
	return dist(_generator.mt);
}

template <> i8 range(i8 _min, i8 _max)
{
	return range(s_globalRandomGenerator, _min, _max);
}

template <> i8 range(RandomGenerator& _generator, i8 _min, i8 _max)
{
	YAE_ASSERT(_min <= _max);
	std::uniform_int_distribution<i16> dist(_min, _max);
	return dist(_generator.mt);
}

template <> u64 range(u64 _min, u64 _max)
{
	return range(s_globalRandomGenerator, _min, _max);
}

template <> u64 range(RandomGenerator& _generator, u64 _min, u64 _max)
{
	YAE_ASSERT(_min <= _max);
	std::uniform_int_distribution<u64> dist(_min, _max);
	return dist(_generator.mt);
}

template <> u32 range(u32 _min, u32 _max)
{
	return range(s_globalRandomGenerator, _min, _max);
}

template <> u32 range(RandomGenerator& _generator, u32 _min, u32 _max)
{
	YAE_ASSERT(_min <= _max);
	std::uniform_int_distribution<u32> dist(_min, _max);
	return dist(_generator.mt);
}

template <> u16 range(u16 _min, u16 _max)
{
	return range(s_globalRandomGenerator, _min, _max);
}

template <> u16 range(RandomGenerator& _generator, u16 _min, u16 _max)
{
	YAE_ASSERT(_min <= _max);
	std::uniform_int_distribution<u16> dist(_min, _max);
	return dist(_generator.mt);
}

} // namespace random
} // namespace yae
