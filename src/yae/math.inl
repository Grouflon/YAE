
#include <cmath>
#include <algorithm>

namespace yae {

i8     abs(i8 _value)     { return std::abs(_value); }
i16    abs(i16 _value)    { return std::abs(_value); }
i32    abs(i32 _value)    { return std::abs(_value); }
i64    abs(i64 _value)    { return std::abs(_value); }
float  abs(float _value)  { return std::abs(_value); }
double abs(double _value) { return std::abs(_value); }

u8     min(u8 _a, u8 _b)         { return std::min(_a, _b); }
u16    min(u16 _a, u16 _b)       { return std::min(_a, _b); }
u32    min(u32 _a, u32 _b)       { return std::min(_a, _b); }
u64    min(u64 _a, u64 _b)       { return std::min(_a, _b); }
i8     min(i8 _a, i8 _b)         { return std::min(_a, _b); }
i16    min(i16 _a, i16 _b)       { return std::min(_a, _b); }
i32    min(i32 _a, i32 _b)       { return std::min(_a, _b); }
i64    min(i64 _a, i64 _b)       { return std::min(_a, _b); }
float  min(float _a, float _b)   { return std::min(_a, _b); }
double min(double _a, double _b) { return std::min(_a, _b); }

u8     max(u8 _a, u8 _b)         { return std::max(_a, _b); }
u16    max(u16 _a, u16 _b)       { return std::max(_a, _b); }
u32    max(u32 _a, u32 _b)       { return std::max(_a, _b); }
u64    max(u64 _a, u64 _b)       { return std::max(_a, _b); }
i8     max(i8 _a, i8 _b)         { return std::max(_a, _b); }
i16    max(i16 _a, i16 _b)       { return std::max(_a, _b); }
i32    max(i32 _a, i32 _b)       { return std::max(_a, _b); }
i64    max(i64 _a, i64 _b)       { return std::max(_a, _b); }
float  max(float _a, float _b)   { return std::max(_a, _b); }
double max(double _a, double _b) { return std::max(_a, _b); }

float clamp(float _value, float _min, float _max)
{
	YAE_ASSERT(_min <= _max);
	return max(min(_value, _max), _min);
}

float mod(float _a, float _b)
{
	return std::fmod(_a, _b);
}

float positiveMod(float _a, float _b)
{
	return mod(mod(_a, _b) + _b, _b);
}

bool isZero(float _value, float _threshold)
{
	return abs(_value) <= _threshold;
}

bool isZero(double _value, double _threshold)
{
	return abs(_value) <= _threshold;
}

} // namespace yae
