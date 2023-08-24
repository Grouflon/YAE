#pragma once

#include <core/types.h>

namespace yae {
namespace math {

inline i8     abs(i8 _value);
inline i16    abs(i16 _value);
inline i32    abs(i32 _value);
inline i64    abs(i64 _value);
inline float  abs(float _value);
inline double abs(double _value);

inline u8     min(u8 _a, u8 _b);
inline u16    min(u16 _a, u16 _b);
inline u32    min(u32 _a, u32 _b);
inline u64    min(u64 _a, u64 _b);
inline i8     min(i8 _a, i8 _b);
inline i16    min(i16 _a, i16 _b);
inline i32    min(i32 _a, i32 _b);
inline i64    min(i64 _a, i64 _b);
inline float  min(float _a, float _b);
inline double min(double _a, double _b);

inline u8     max(u8 _a, u8 _b);
inline u16    max(u16 _a, u16 _b);
inline u32    max(u32 _a, u32 _b);
inline u64    max(u64 _a, u64 _b);
inline i8     max(i8 _a, i8 _b);
inline i16    max(i16 _a, i16 _b);
inline i32    max(i32 _a, i32 _b);
inline i64    max(i64 _a, i64 _b);
inline float  max(float _a, float _b);
inline double max(double _a, double _b);

inline i8 sign(i8 _value);
inline i16 sign(i16 _value);
inline i32 sign(i32 _value);
inline i64 sign(i64 _value);
inline float sign(float _value);
inline double sign(double _value);

inline float clamp(float _value, float _min, float _max);

inline float mod(float _a, float _b);
inline float positiveMod(float _a, float _b);

inline bool isZero(float _value, float _threshold = SMALL_NUMBER);
inline bool isNan(float _value);
inline bool isFinite(float _value);
inline bool isZero(double _value, double _threshold = SMALL_NUMBER);
inline bool isNan(double _value);
inline bool isFinite(double _value);

template <typename T>
inline bool isEqual(const T& _a, const T& _b, float _threshold = SMALL_NUMBER)
{
	return isZero(_a - _b, _threshold);
}

} // namespace math
} // namespace yae

#include "math.inl"
