#pragma once

#include <yae/types.h>

namespace yae {

struct RandomGenerator;

namespace random {

YAE_API void setSeed(u32 _seed);
YAE_API void setSeed(RandomGenerator& _generator, u32 _seed);

YAE_API float range01();
YAE_API float range01(RandomGenerator& _generator);

template <typename T> T range(T _min, T _max) { static_assert(false); }
template <typename T> T range(RandomGenerator& _generator, T _min, T _max) { static_assert(false); }

template <> YAE_API float range(float _min, float _max);
template <> YAE_API float range(RandomGenerator& _generator, float _min, float _max);
template <> YAE_API double range(double _min, double _max);
template <> YAE_API double range(RandomGenerator& _generator, double _min, double _max);

template <> YAE_API i64 range(i64 _min, i64 _max);
template <> YAE_API i64 range(RandomGenerator& _generator, i64 _min, i64 _max);
template <> YAE_API i32 range(i32 _min, i32 _max);
template <> YAE_API i32 range(RandomGenerator& _generator, i32 _min, i32 _max);
template <> YAE_API i16 range(i16 _min, i16 _max);
template <> YAE_API i16 range(RandomGenerator& _generator, i16 _min, i16 _max);
template <> YAE_API i8 range(i8 _min, i8 _max);
template <> YAE_API i8 range(RandomGenerator& _generator, i8 _min, i8 _max);

template <> YAE_API u64 range(u64 _mun, u64 _max);
template <> YAE_API u64 range(RandomGenerator& _generator, u64 _mun, u64 _max);
template <> YAE_API u32 range(u32 _mun, u32 _max);
template <> YAE_API u32 range(RandomGenerator& _generator, u32 _mun, u32 _max);
template <> YAE_API u16 range(u16 _mun, u16 _max);
template <> YAE_API u16 range(RandomGenerator& _generator, u16 _mun, u16 _max);
// @NOTE(remi): u8 is not supported by std::uniform_int_distribution

} // namespace random
} // namespace yae
