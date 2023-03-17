namespace yae {

// - Vector2 -
// Ctors
inline Vector2::Vector2() {}
inline Vector2::Vector2(float _value) : x(_value), y(_value) {}
inline Vector2::Vector2(float _x, float _y) : x(_x), y(_y) {}

// Operators
// -- Component accesses --
inline float& Vector2::operator[](size_t _i)
{
	YAE_ASSERT(_i >= 0 && _i < 2);
	switch(_i)
	{
		default:
		case 0: return x;
		case 1: return y;
	}
}

inline const float& Vector2::operator[](size_t _i) const
{
	YAE_ASSERT(_i >= 0 && _i < 2);
	switch(_i)
	{
		default:
		case 0: return x;
		case 1: return y;
	}
}

// -- Unary arithmetic operators --
inline Vector2& Vector2::operator+=(float _s)
{
	this->x += _s;
	this->y += _s;
	return *this;
}

inline Vector2& Vector2::operator+=(const Vector2& _v)
{
	this->x += _v.x;
	this->y += _v.y;
	return *this;
}

inline Vector2& Vector2::operator-=(float _s)
{
	this->x -= _s;
	this->y -= _s;
	return *this;
}

inline Vector2& Vector2::operator-=(const Vector2& _v)
{
	this->x -= _v.x;
	this->y -= _v.y;
	return *this;
}

inline Vector2& Vector2::operator*=(float _s)
{
	this->x *= _s;
	this->y *= _s;
	return *this;
}

inline Vector2& Vector2::operator*=(const Vector2& _v)
{
	this->x *= _v.x;
	this->y *= _v.y;
	return *this;
}

inline Vector2& Vector2::operator/=(float _s)
{
	YAE_ASSERT(std::abs(_s) > VERY_SMALL_NUMBER);
	this->x /= _s;
	this->y /= _s;
	return *this;
}

inline Vector2& Vector2::operator/=(const Vector2& _v)
{
	YAE_ASSERT(std::abs(_v.x) > VERY_SMALL_NUMBER);
	YAE_ASSERT(std::abs(_v.y) > VERY_SMALL_NUMBER);
	this->x /= _v.x;
	this->y /= _v.y;
	return *this;	
}

// -- Unary operators --
inline Vector2 operator+(const Vector2& _v)
{
	return _v;
}

inline Vector2 operator-(const Vector2& _v)
{
	return Vector2(
		-_v.x,
		-_v.y
	);
}

// -- Binary operators --
inline Vector2 operator+(const Vector2& _v, float _s)
{
	return Vector2(
		_v.x + _s,
		_v.y + _s
	);
}

inline Vector2 operator+(float _s, const Vector2& _v)
{
	return Vector2(
		_s + _v.x,
		_s + _v.y
	);
}

inline Vector2 operator+(const Vector2& _v1, const Vector2& _v2)
{
	return Vector2(
		_v1.x + _v2.x,
		_v1.y + _v2.y
	);
}

inline Vector2 operator-(const Vector2& _v, float _s)
{
	return Vector2(
		_v.x - _s,
		_v.y - _s
	);
}

inline Vector2 operator-(float _s, const Vector2& _v)
{
	return Vector2(
		_s - _v.x,
		_s - _v.y
	);
}

inline Vector2 operator-(const Vector2& _v1, const Vector2& _v2)
{
	return Vector2(
		_v1.x - _v2.x,
		_v1.y - _v2.y
	);
}

inline Vector2 operator*(const Vector2& _v, float _s)
{
	return Vector2(
		_v.x * _s,
		_v.y * _s
	);
}

inline Vector2 operator*(float _s, const Vector2& _v)
{
	return Vector2(
		_s * _v.x,
		_s * _v.y
	);
}

inline Vector2 operator*(const Vector2& _v1, const Vector2& _v2)
{
	return Vector2(
		_v1.x * _v2.x,
		_v1.y * _v2.y
	);
}

inline Vector2 operator/(const Vector2& _v, float _s)
{
	return Vector2(
		_v.x / _s,
		_v.y / _s
	);
}

inline Vector2 operator/(float _s, const Vector2& _v)
{
	return Vector2(
		_s / _v.x,
		_s / _v.y
	);
}

inline Vector2 operator/(const Vector2& _v1, const Vector2& _v2)
{
	return Vector2(
		_v1.x / _v2.x,
		_v1.y / _v2.y
	);
}

// -- Boolean operators --
inline bool operator==(const Vector2& _v1, const Vector2& _v2)
{
	return 
		_v1.x == _v2.x &&
		_v1.y == _v2.y;
}

inline bool operator!=(const Vector2& _v1, const Vector2& _v2)
{
	return !(_v1 == _v2);
}

// - Vector3 -
// Ctors
inline Vector3::Vector3() {}
inline Vector3::Vector3(float _value) : x(_value), y(_value), z(_value) {}
inline Vector3::Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
inline Vector3::Vector3(const Vector2& _xy, float _z) : x(_xy.x), y(_xy.y), z(_z) {}

// Operators
// -- Component accesses --
inline float& Vector3::operator[](size_t _i)
{
	YAE_ASSERT(_i >= 0 && _i < 3);
	switch(_i)
	{
		default:
		case 0: return x;
		case 1: return y;
		case 2: return z;
	}
}

inline const float& Vector3::operator[](size_t _i) const
{
	YAE_ASSERT(_i >= 0 && _i < 3);
	switch(_i)
	{
		default:
		case 0: return x;
		case 1: return y;
		case 2: return z;
	}
}

// -- Unary arithmetic operators --
inline Vector3& Vector3::operator+=(float _s)
{
	this->x += _s;
	this->y += _s;
	this->z += _s;
	return *this;
}

inline Vector3& Vector3::operator+=(const Vector3& _v)
{
	this->x += _v.x;
	this->y += _v.y;
	this->z += _v.z;
	return *this;
}

inline Vector3& Vector3::operator-=(float _s)
{
	this->x -= _s;
	this->y -= _s;
	this->z -= _s;
	return *this;
}

inline Vector3& Vector3::operator-=(const Vector3& _v)
{
	this->x -= _v.x;
	this->y -= _v.y;
	this->y -= _v.z;
	return *this;
}

inline Vector3& Vector3::operator*=(float _s)
{
	this->x *= _s;
	this->y *= _s;
	this->z *= _s;
	return *this;
}

inline Vector3& Vector3::operator*=(const Vector3& _v)
{
	this->x *= _v.x;
	this->y *= _v.y;
	this->z *= _v.z;
	return *this;
}

inline Vector3& Vector3::operator/=(float _s)
{
	YAE_ASSERT(std::abs(_s) > VERY_SMALL_NUMBER);
	this->x /= _s;
	this->y /= _s;
	this->z /= _s;
	return *this;
}

inline Vector3& Vector3::operator/=(const Vector3& _v)
{
	YAE_ASSERT(std::abs(_v.x) > VERY_SMALL_NUMBER);
	YAE_ASSERT(std::abs(_v.y) > VERY_SMALL_NUMBER);
	YAE_ASSERT(std::abs(_v.z) > VERY_SMALL_NUMBER);
	this->x /= _v.x;
	this->y /= _v.y;
	this->z /= _v.z;
	return *this;	
}

// -- Unary operators --
inline Vector3 operator+(const Vector3& _v)
{
	return _v;
}

inline Vector3 operator-(const Vector3& _v)
{
	return Vector3(
		-_v.x,
		-_v.y,
		-_v.z
	);
}

// -- Binary operators --
inline Vector3 operator+(const Vector3& _v, float _s)
{
	return Vector3(
		_v.x + _s,
		_v.y + _s,
		_v.z + _s
	);
}

inline Vector3 operator+(float _s, const Vector3& _v)
{
	return Vector3(
		_s + _v.x,
		_s + _v.y,
		_s + _v.z
	);
}

inline Vector3 operator+(const Vector3& _v1, const Vector3& _v2)
{
	return Vector3(
		_v1.x + _v2.x,
		_v1.y + _v2.y,
		_v1.z + _v2.z
	);
}

inline Vector3 operator-(const Vector3& _v, float _s)
{
	return Vector3(
		_v.x - _s,
		_v.y - _s,
		_v.z - _s
	);
}

inline Vector3 operator-(float _s, const Vector3& _v)
{
	return Vector3(
		_s - _v.x,
		_s - _v.y,
		_s - _v.z
	);
}

inline Vector3 operator-(const Vector3& _v1, const Vector3& _v2)
{
	return Vector3(
		_v1.x - _v2.x,
		_v1.y - _v2.y,
		_v1.z - _v2.z
	);
}

inline Vector3 operator*(const Vector3& _v, float _s)
{
	return Vector3(
		_v.x * _s,
		_v.y * _s,
		_v.z * _s
	);
}

inline Vector3 operator*(float _s, const Vector3& _v)
{
	return Vector3(
		_s * _v.x,
		_s * _v.y,
		_s * _v.z
	);
}

inline Vector3 operator*(const Vector3& _v1, const Vector3& _v2)
{
	return Vector3(
		_v1.x * _v2.x,
		_v1.y * _v2.y,
		_v1.z * _v2.z
	);
}

inline Vector3 operator/(const Vector3& _v, float _s)
{
	return Vector3(
		_v.x / _s,
		_v.y / _s,
		_v.z / _s
	);
}

inline Vector3 operator/(float _s, const Vector3& _v)
{
	return Vector3(
		_s / _v.x,
		_s / _v.y,
		_s / _v.z
	);
}

inline Vector3 operator/(const Vector3& _v1, const Vector3& _v2)
{
	return Vector3(
		_v1.x / _v2.x,
		_v1.y / _v2.y,
		_v1.z / _v2.z
	);
}

// -- Boolean operators --
inline bool operator==(const Vector3& _v1, const Vector3& _v2)
{
	return 
		_v1.x == _v2.x &&
		_v1.y == _v2.y &&
		_v1.z == _v2.z;
}

inline bool operator!=(const Vector3& _v1, const Vector3& _v2)
{
	return !(_v1 == _v2);
}

// - Vector4 -
// Ctors
inline Vector4::Vector4() {}
inline Vector4::Vector4(float _value) : x(_value), y(_value), z(_value), w(_value) {}
inline Vector4::Vector4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
inline Vector4::Vector4(const Vector2& _xy, float _z, float _w) : x(_xy.x), y(_xy.y), z(_z), w(_w) {}
inline Vector4::Vector4(const Vector3& _xyz, float _w) : x(_xyz.x), y(_xyz.y), z(_xyz.z), w(_w) {}

// Operators
// -- Component accesses --
inline float& Vector4::operator[](size_t _i)
{
	YAE_ASSERT(_i >= 0 && _i < 4);
	switch(_i)
	{
		default:
		case 0: return x;
		case 1: return y;
		case 2: return z;
		case 3: return w;
	}
}

inline const float& Vector4::operator[](size_t _i) const
{
	YAE_ASSERT(_i >= 0 && _i < 4);
	switch(_i)
	{
		default:
		case 0: return x;
		case 1: return y;
		case 2: return z;
		case 3: return w;
	}
}

// -- Unary arithmetic operators --
inline Vector4& Vector4::operator+=(float _s)
{
	this->x += _s;
	this->y += _s;
	this->z += _s;
	this->w += _s;
	return *this;
}

inline Vector4& Vector4::operator+=(const Vector4& _v)
{
	this->x += _v.x;
	this->y += _v.y;
	this->z += _v.z;
	this->w += _v.w;
	return *this;
}

inline Vector4& Vector4::operator-=(float _s)
{
	this->x -= _s;
	this->y -= _s;
	this->z -= _s;
	this->w -= _s;
	return *this;
}

inline Vector4& Vector4::operator-=(const Vector4& _v)
{
	this->x -= _v.x;
	this->y -= _v.y;
	this->y -= _v.z;
	this->w -= _v.w;
	return *this;
}

inline Vector4& Vector4::operator*=(float _s)
{
	this->x *= _s;
	this->y *= _s;
	this->z *= _s;
	this->w *= _s;
	return *this;
}

inline Vector4& Vector4::operator*=(const Vector4& _v)
{
	this->x *= _v.x;
	this->y *= _v.y;
	this->z *= _v.z;
	this->w *= _v.w;
	return *this;
}

inline Vector4& Vector4::operator/=(float _s)
{
	YAE_ASSERT(std::abs(_s) > VERY_SMALL_NUMBER);
	this->x /= _s;
	this->y /= _s;
	this->z /= _s;
	this->w /= _s;
	return *this;
}

inline Vector4& Vector4::operator/=(const Vector4& _v)
{
	YAE_ASSERT(std::abs(_v.x) > VERY_SMALL_NUMBER);
	YAE_ASSERT(std::abs(_v.y) > VERY_SMALL_NUMBER);
	YAE_ASSERT(std::abs(_v.z) > VERY_SMALL_NUMBER);
	YAE_ASSERT(std::abs(_v.w) > VERY_SMALL_NUMBER);
	this->x /= _v.x;
	this->y /= _v.y;
	this->z /= _v.z;
	this->w /= _v.w;
	return *this;	
}

// -- Unary operators --
inline Vector4 operator+(const Vector4& _v)
{
	return _v;
}

inline Vector4 operator-(const Vector4& _v)
{
	return Vector4(
		-_v.x,
		-_v.y,
		-_v.z,
		-_v.w
	);
}

// -- Binary operators --
inline Vector4 operator+(const Vector4& _v, float _s)
{
	return Vector4(
		_v.x + _s,
		_v.y + _s,
		_v.z + _s,
		_v.w + _s
	);
}

inline Vector4 operator+(float _s, const Vector4& _v)
{
	return Vector4(
		_s + _v.x,
		_s + _v.y,
		_s + _v.z,
		_s + _v.w
	);
}

inline Vector4 operator+(const Vector4& _v1, const Vector4& _v2)
{
	return Vector4(
		_v1.x + _v2.x,
		_v1.y + _v2.y,
		_v1.z + _v2.z,
		_v1.w + _v2.w
	);
}

inline Vector4 operator-(const Vector4& _v, float _s)
{
	return Vector4(
		_v.x - _s,
		_v.y - _s,
		_v.z - _s,
		_v.w - _s
	);
}

inline Vector4 operator-(float _s, const Vector4& _v)
{
	return Vector4(
		_s - _v.x,
		_s - _v.y,
		_s - _v.z,
		_s - _v.w
	);
}

inline Vector4 operator-(const Vector4& _v1, const Vector4& _v2)
{
	return Vector4(
		_v1.x - _v2.x,
		_v1.y - _v2.y,
		_v1.z - _v2.z,
		_v1.w - _v2.w
	);
}

inline Vector4 operator*(const Vector4& _v, float _s)
{
	return Vector4(
		_v.x * _s,
		_v.y * _s,
		_v.z * _s,
		_v.w * _s
	);
}

inline Vector4 operator*(float _s, const Vector4& _v)
{
	return Vector4(
		_s * _v.x,
		_s * _v.y,
		_s * _v.z,
		_s * _v.w
	);
}

inline Vector4 operator*(const Vector4& _v1, const Vector4& _v2)
{
	return Vector4(
		_v1.x * _v2.x,
		_v1.y * _v2.y,
		_v1.z * _v2.z,
		_v1.w * _v2.w
	);
}

inline Vector4 operator/(const Vector4& _v, float _s)
{
	YAE_ASSERT(std::abs(_s) > VERY_SMALL_NUMBER);
	return Vector4(
		_v.x / _s,
		_v.y / _s,
		_v.z / _s,
		_v.w / _s
	);

}

inline Vector4 operator/(float _s, const Vector4& _v)
{
	YAE_ASSERT(std::abs(_v.x) > VERY_SMALL_NUMBER);
	YAE_ASSERT(std::abs(_v.y) > VERY_SMALL_NUMBER);
	YAE_ASSERT(std::abs(_v.z) > VERY_SMALL_NUMBER);
	YAE_ASSERT(std::abs(_v.w) > VERY_SMALL_NUMBER);
	return Vector4(
		_s / _v.x,
		_s / _v.y,
		_s / _v.z,
		_s / _v.w
	);
}

inline Vector4 operator/(const Vector4& _v1, const Vector4& _v2)
{
	return Vector4(
		_v1.x / _v2.x,
		_v1.y / _v2.y,
		_v1.z / _v2.z,
		_v1.w / _v2.w
	);
}

// -- Boolean operators --
inline bool operator==(const Vector4& _v1, const Vector4& _v2)
{
	return 
		_v1.x == _v2.x &&
		_v1.y == _v2.y &&
		_v1.z == _v2.z &&
		_v1.w == _v2.w;
}

inline bool operator!=(const Vector4& _v1, const Vector4& _v2)
{
	return !(_v1 == _v2);
}

// - Quaternion -
inline Quaternion::Quaternion() {}
inline Quaternion::Quaternion(float _value) : x(_value), y(_value), z(_value), w(_value) {}
inline Quaternion::Quaternion(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}

// Operators
// -- Component accesses --
inline float& Quaternion::operator[](size_t _i)
{
	YAE_ASSERT(_i >= 0 && _i < 4);
	switch(_i)
	{
		default:
		case 0: return x;
		case 1: return y;
		case 2: return z;
		case 3: return w;
	}
}

inline const float& Quaternion::operator[](size_t _i) const
{
	YAE_ASSERT(_i >= 0 && _i < 4);
	switch(_i)
	{
		default:
		case 0: return x;
		case 1: return y;
		case 2: return z;
		case 3: return w;
	}
}

// -- Unary arithmetic operators --
inline Quaternion& Quaternion::operator*=(float _s)
{
	this->x *= _s;
	this->y *= _s;
	this->z *= _s;
	this->w *= _s;
	return *this;
}

inline Quaternion& Quaternion::operator*=(const Quaternion& _q)
{
	Quaternion const p(*this);
	Quaternion const q(_q);

	this->x = p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y;
	this->y = p.w * q.y + p.y * q.w + p.z * q.x - p.x * q.z;
	this->z = p.w * q.z + p.z * q.w + p.x * q.y - p.y * q.x;
	this->w = p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z;
	return *this;
}

inline Quaternion& Quaternion::operator/=(float _s)
{
	this->x /= _s;
	this->y /= _s;
	this->z /= _s;
	this->w /= _s;
	return *this;
}

// -- Binary operators --
inline Quaternion operator*(const Quaternion& _q, float _s)
{
	return Quaternion(_q) *= _s;
}

inline Quaternion operator*(const Quaternion& _q1, const Quaternion& _q2)
{
	return Quaternion(_q1) *= _q2;
}

inline Vector3 operator*(const Quaternion& _q, const Vector3& _v)
{
	Vector3 const quatVector(_q.x, _q.y, _q.z);
	Vector3 const uv = Vector3(
		quatVector.y * _v.z - _v.y * quatVector.z,
		quatVector.z * _v.x - _v.z * quatVector.x,
		quatVector.x * _v.y - _v.x * quatVector.y
	);
	Vector3 const uuv = Vector3(
		quatVector.y * uv.z - uv.y * quatVector.z,
		quatVector.z * uv.x - uv.z * quatVector.x,
		quatVector.x * uv.y - uv.x * quatVector.y
	);

	return _v + ((uv * _q.w) + uuv) * 2.f;
}

inline Quaternion operator/(const Quaternion& _q, float _s)
{
	return Quaternion(_q) /= _s;
}

// -- Boolean operators --
inline bool operator==(const Quaternion& _q1, const Quaternion& _q2)
{
	return 
		_q1.x == _q2.x &&
		_q1.y == _q2.y &&
		_q1.z == _q2.z &&
		_q1.w == _q2.w;
}

inline bool operator!=(const Quaternion& _q1, const Quaternion& _q2)
{
	return !(_q1 == _q2);
}

// - Matrix3 -
// Ctors
inline Matrix3::Matrix3() {}
inline Matrix3::Matrix3(float _value)
{
	m[0][0] = _value; m[0][1] = _value; m[0][2] = _value;
	m[1][0] = _value; m[1][1] = _value; m[1][2] = _value;
	m[2][0] = _value; m[2][1] = _value; m[2][2] = _value;
}
inline Matrix3::Matrix3(float _m00, float _m01, float _m02,
	float _m10, float _m11, float _m12,
	float _m20, float _m21, float _m22)
{
	m[0][0] = _m00; m[0][1] = _m01; m[0][2] = _m02;
	m[1][0] = _m10; m[1][1] = _m11; m[1][2] = _m12;
	m[2][0] = _m20; m[2][1] = _m21; m[2][2] = _m22;
}

inline Matrix3::Matrix3(const Vector3& _m0,
	const Vector3& _m1,
	const Vector3& _m2)
{
	m[0] = _m0;
	m[1] = _m1;
	m[2] = _m2;
}

// Operators
// -- Component accesses --
inline Vector3& Matrix3::operator[](size_t _i)
{
	YAE_ASSERT(_i >= 0 && _i < 3);
	switch(_i)
	{
		default:
		case 0: return m[0];
		case 1: return m[1];
		case 2: return m[2];
	}
}

inline const Vector3& Matrix3::operator[](size_t _i) const
{
	YAE_ASSERT(_i >= 0 && _i < 3);
	switch(_i)
	{
		default:
		case 0: return m[0];
		case 1: return m[1];
		case 2: return m[2];
	}
}

// -- Unary arithmetic operators --
inline Matrix3& Matrix3::operator+=(float _s)
{
	this->m[0] += _s;
	this->m[1] += _s;
	this->m[2] += _s;
	return *this;
}

inline Matrix3& Matrix3::operator+=(const Matrix3& _m)
{
	this->m[0] += _m[0];
	this->m[1] += _m[1];
	this->m[2] += _m[2];
	return *this;
}

inline Matrix3& Matrix3::operator-=(float _s)
{
	this->m[0] -= _s;
	this->m[1] -= _s;
	this->m[2] -= _s;
	return *this;
}

inline Matrix3& Matrix3::operator-=(const Matrix3& _m)
{
	this->m[0] -= _m[0];
	this->m[1] -= _m[1];
	this->m[2] -= _m[2];
	return *this;
}

inline Matrix3& Matrix3::operator*=(float _s)
{
	this->m[0] *= _s;
	this->m[1] *= _s;
	this->m[2] *= _s;
	return *this;
}

inline Matrix3& Matrix3::operator*=(const Matrix3& _m)
{
	return (*this = *this * _m);
}

inline Matrix3& Matrix3::operator/=(float _s)
{
	this->m[0] /= _s;
	this->m[1] /= _s;
	this->m[2] /= _s;
	return *this;
}

inline Matrix3& Matrix3::operator/=(const Matrix3& _m)
{
	return (*this = *this / _m);
}
// -- Unary operators --
inline Matrix3 operator+(const Matrix3& _m)
{
	return _m;
}

inline Matrix3 operator-(const Matrix3& _m)
{
	return Matrix3(
		-_m[0],
		-_m[1],
		-_m[2]
	);
}

// -- Binary operators --
inline Matrix3 operator+(const Matrix3& _m, float _s)
{
	return Matrix3(
		_m[0] + _s,
		_m[1] + _s,
		_m[2] + _s
	);
}

inline Matrix3 operator+(float _s, const Matrix3& _m)
{
	return Matrix3(
		_s + _m[0],
		_s + _m[1],
		_s + _m[2]
	);
}

inline Matrix3 operator+(const Matrix3& _m1, const Matrix3& _m2)
{
	return Matrix3(
		_m1[0] + _m2[0],
		_m1[1] + _m2[1],
		_m1[2] + _m2[2]
	);
}

inline Matrix3 operator-(const Matrix3& _m, float _s)
{
	return Matrix3(
		_m[0] - _s,
		_m[1] - _s,
		_m[2] - _s
	);
}

inline Matrix3 operator-(float _s, const Matrix3& _m)
{
	return Matrix3(
		_s - _m[0],
		_s - _m[1],
		_s - _m[2]
	);
}

inline Matrix3 operator-(const Matrix3& _m1, const Matrix3& _m2)
{
	return Matrix3(
		_m1[0] - _m2[0],
		_m1[1] - _m2[1],
		_m1[2] - _m2[2]
	);
}

inline Matrix3 operator*(const Matrix3& _m, float _s)
{
	return Matrix3(
		_m[0] * _s,
		_m[1] * _s,
		_m[2] * _s
	);
}

inline Matrix3 operator*(float _s, const Matrix3& _m)
{
	return Matrix3(
		_s * _m[0],
		_s * _m[1],
		_s * _m[2]
	);
}

inline Matrix3 operator*(const Matrix3& _m1, const Matrix3& _m2)
{
	float const SrcA00 = _m1[0][0];
	float const SrcA01 = _m1[0][1];
	float const SrcA02 = _m1[0][2];
	float const SrcA10 = _m1[1][0];
	float const SrcA11 = _m1[1][1];
	float const SrcA12 = _m1[1][2];
	float const SrcA20 = _m1[2][0];
	float const SrcA21 = _m1[2][1];
	float const SrcA22 = _m1[2][2];

	float const SrcB00 = _m2[0][0];
	float const SrcB01 = _m2[0][1];
	float const SrcB02 = _m2[0][2];
	float const SrcB10 = _m2[1][0];
	float const SrcB11 = _m2[1][1];
	float const SrcB12 = _m2[1][2];
	float const SrcB20 = _m2[2][0];
	float const SrcB21 = _m2[2][1];
	float const SrcB22 = _m2[2][2];

	Matrix3 Result;
	Result[0][0] = SrcA00 * SrcB00 + SrcA10 * SrcB01 + SrcA20 * SrcB02;
	Result[0][1] = SrcA01 * SrcB00 + SrcA11 * SrcB01 + SrcA21 * SrcB02;
	Result[0][2] = SrcA02 * SrcB00 + SrcA12 * SrcB01 + SrcA22 * SrcB02;
	Result[1][0] = SrcA00 * SrcB10 + SrcA10 * SrcB11 + SrcA20 * SrcB12;
	Result[1][1] = SrcA01 * SrcB10 + SrcA11 * SrcB11 + SrcA21 * SrcB12;
	Result[1][2] = SrcA02 * SrcB10 + SrcA12 * SrcB11 + SrcA22 * SrcB12;
	Result[2][0] = SrcA00 * SrcB20 + SrcA10 * SrcB21 + SrcA20 * SrcB22;
	Result[2][1] = SrcA01 * SrcB20 + SrcA11 * SrcB21 + SrcA21 * SrcB22;
	Result[2][2] = SrcA02 * SrcB20 + SrcA12 * SrcB21 + SrcA22 * SrcB22;
	return Result;
}

inline Vector3 operator*(const Matrix3& _m, const Vector3& _v)
{
	return Vector3(
		_m[0][0] * _v.x + _m[1][0] * _v.y + _m[2][0] * _v.z,
		_m[0][1] * _v.x + _m[1][1] * _v.y + _m[2][1] * _v.z,
		_m[0][2] * _v.x + _m[1][2] * _v.y + _m[2][2] * _v.z
	);
}

inline Vector3 operator*(const Matrix3& _m, const Vector2& _v)
{
	return _m * Vector3(_v);
}

inline Matrix3 operator/(const Matrix3& _m, float _s)
{
	return Matrix3(
		_m[0] / _s,
		_m[1] / _s,
		_m[2] / _s
	);
}

inline Matrix3 operator/(float _s, const Matrix3& _m)
{
	return Matrix3(
		_s / _m[0],
		_s / _m[1],
		_s / _m[2]
	);
}

inline Matrix3 operator/(const Matrix3& _m1, const Matrix3& _m2)
{
	float oneOverDeterminant = 1.f / (
		+ _m2[0][0] * (_m2[1][1] * _m2[2][2] - _m2[2][1] * _m2[1][2])
		- _m2[1][0] * (_m2[0][1] * _m2[2][2] - _m2[2][1] * _m2[0][2])
		+ _m2[2][0] * (_m2[0][1] * _m2[1][2] - _m2[1][1] * _m2[0][2]));

	Matrix3 m2Inverse;
	m2Inverse[0][0] = + (_m2[1][1] * _m2[2][2] - _m2[2][1] * _m2[1][2]) * oneOverDeterminant;
	m2Inverse[1][0] = - (_m2[1][0] * _m2[2][2] - _m2[2][0] * _m2[1][2]) * oneOverDeterminant;
	m2Inverse[2][0] = + (_m2[1][0] * _m2[2][1] - _m2[2][0] * _m2[1][1]) * oneOverDeterminant;
	m2Inverse[0][1] = - (_m2[0][1] * _m2[2][2] - _m2[2][1] * _m2[0][2]) * oneOverDeterminant;
	m2Inverse[1][1] = + (_m2[0][0] * _m2[2][2] - _m2[2][0] * _m2[0][2]) * oneOverDeterminant;
	m2Inverse[2][1] = - (_m2[0][0] * _m2[2][1] - _m2[2][0] * _m2[0][1]) * oneOverDeterminant;
	m2Inverse[0][2] = + (_m2[0][1] * _m2[1][2] - _m2[1][1] * _m2[0][2]) * oneOverDeterminant;
	m2Inverse[1][2] = - (_m2[0][0] * _m2[1][2] - _m2[1][0] * _m2[0][2]) * oneOverDeterminant;
	m2Inverse[2][2] = + (_m2[0][0] * _m2[1][1] - _m2[1][0] * _m2[0][1]) * oneOverDeterminant;

	return _m1 * m2Inverse;
}


// -- Boolean operators --
inline bool operator==(const Matrix3& _m1, const Matrix3& _m2)
{
	return (_m1[0] == _m2[0]) && (_m1[1] == _m2[1]) && (_m1[2] == _m2[2]);
}

inline bool operator!=(const Matrix3& _m1, const Matrix3& _m2)
{
	return (_m1[0] != _m2[0]) || (_m1[1] != _m2[1]) || (_m1[2] != _m2[2]);
}

// - Matrix4 -
inline Matrix4::Matrix4() {}
inline Matrix4::Matrix4(float _value)
{
	m[0][0] = _value; m[0][1] = _value; m[0][2] = _value; m[0][3] = _value;
	m[1][0] = _value; m[1][1] = _value; m[1][2] = _value; m[1][3] = _value;
	m[2][0] = _value; m[2][1] = _value; m[2][2] = _value; m[2][3] = _value;
	m[3][0] = _value; m[3][1] = _value; m[3][2] = _value; m[3][3] = _value;
}
inline Matrix4::Matrix4(float _m00, float _m01, float _m02, float _m03,
		float _m10, float _m11, float _m12, float _m13,
		float _m20, float _m21, float _m22, float _m23,
		float _m30, float _m31, float _m32, float _m33)
{
	m[0][0] = _m00; m[0][1] = _m01; m[0][2] = _m02; m[0][3] = _m03;
	m[1][0] = _m10; m[1][1] = _m11; m[1][2] = _m12; m[1][3] = _m13;
	m[2][0] = _m20; m[2][1] = _m21; m[2][2] = _m22; m[2][3] = _m23;
	m[3][0] = _m30; m[3][1] = _m31; m[3][2] = _m32; m[3][3] = _m33;
}
inline Matrix4::Matrix4(const Vector4& _m0,
		const Vector4& _m1,
		const Vector4& _m2,
		const Vector4& _m3)
{
	m[0] = _m0;
	m[1] = _m1;
	m[2] = _m2;
	m[3] = _m3;
}

// Operators
// -- Component accesses --
inline Vector4& Matrix4::operator[](size_t _i)
{
	YAE_ASSERT(_i >= 0 && _i < 4);
	switch(_i)
	{
		default:
		case 0: return m[0];
		case 1: return m[1];
		case 2: return m[2];
		case 3: return m[3];
	}
}

inline const Vector4& Matrix4::operator[](size_t _i) const
{
	YAE_ASSERT(_i >= 0 && _i < 4);
	switch(_i)
	{
		default:
		case 0: return m[0];
		case 1: return m[1];
		case 2: return m[2];
		case 3: return m[3];
	}
}

// -- Unary arithmetic operators --
inline Matrix4& Matrix4::operator+=(float _s)
{
	this->m[0] += _s;
	this->m[1] += _s;
	this->m[2] += _s;
	this->m[3] += _s;
	return *this;
}

inline Matrix4& Matrix4::operator+=(const Matrix4& _m)
{
	this->m[0] += _m[0];
	this->m[1] += _m[1];
	this->m[2] += _m[2];
	this->m[3] += _m[3];
	return *this;
}

inline Matrix4& Matrix4::operator-=(float _s)
{
	this->m[0] -= _s;
	this->m[1] -= _s;
	this->m[2] -= _s;
	this->m[3] -= _s;
	return *this;
}

inline Matrix4& Matrix4::operator-=(const Matrix4& _m)
{
	this->m[0] -= _m[0];
	this->m[1] -= _m[1];
	this->m[2] -= _m[2];
	this->m[3] -= _m[3];
	return *this;
}

inline Matrix4& Matrix4::operator*=(float _s)
{
	this->m[0] *= _s;
	this->m[1] *= _s;
	this->m[2] *= _s;
	this->m[3] *= _s;
	return *this;
}

inline Matrix4& Matrix4::operator*=(const Matrix4& _m)
{
	return (*this = *this * _m);
}

inline Matrix4& Matrix4::operator/=(float _s)
{
	this->m[0] /= _s;
	this->m[1] /= _s;
	this->m[2] /= _s;
	this->m[3] /= _s;
	return *this;
}

inline Matrix4& Matrix4::operator/=(const Matrix4& _m)
{
	return (*this = *this / _m);
}

// -- Unary operators --
inline Matrix4 operator+(const Matrix4& _m)
{
	return _m;
}

inline Matrix4 operator-(const Matrix4& _m)
{
	return Matrix4(
		-_m[0],
		-_m[1],
		-_m[2],
		-_m[3]
	);
}

// -- Binary operators --
inline Matrix4 operator+(const Matrix4& _m, float _s)
{
	return Matrix4(
		_m[0] + _s,
		_m[1] + _s,
		_m[2] + _s,
		_m[3] + _s
	);
}

inline Matrix4 operator+(float _s, const Matrix4& _m)
{
	return Matrix4(
		_s + _m[0],
		_s + _m[1],
		_s + _m[2],
		_s + _m[3]
	);
}

inline Matrix4 operator+(const Matrix4& _m1, const Matrix4& _m2)
{
	return Matrix4(
		_m1[0] + _m2[0],
		_m1[1] + _m2[1],
		_m1[2] + _m2[2],
		_m1[3] + _m2[3]
	);
}

inline Matrix4 operator-(const Matrix4& _m, float _s)
{
	return Matrix4(
		_m[0] - _s,
		_m[1] - _s,
		_m[2] - _s,
		_m[3] - _s
	);
}

inline Matrix4 operator-(float _s, const Matrix4& _m)
{
	return Matrix4(
		_s - _m[0],
		_s - _m[1],
		_s - _m[2],
		_s - _m[3]
	);
}

inline Matrix4 operator-(const Matrix4& _m1, const Matrix4& _m2)
{
	return Matrix4(
		_m1[0] - _m2[0],
		_m1[1] - _m2[1],
		_m1[2] - _m2[2],
		_m1[3] - _m2[3]
	);
}

inline Matrix4 operator*(const Matrix4& _m, float _s)
{
	return Matrix4(
		_m[0] * _s,
		_m[1] * _s,
		_m[2] * _s,
		_m[3] * _s
	);
}

inline Matrix4 operator*(float _s, const Matrix4& _m)
{
	return Matrix4(
		_s * _m[0],
		_s * _m[1],
		_s * _m[2],
		_s * _m[3]
	);
}

inline Matrix4 operator*(const Matrix4& _m1, const Matrix4& _m2)
{
	Vector4 const SrcA0 = _m1[0];
	Vector4 const SrcA1 = _m1[1];
	Vector4 const SrcA2 = _m1[2];
	Vector4 const SrcA3 = _m1[3];

	Vector4 const SrcB0 = _m2[0];
	Vector4 const SrcB1 = _m2[1];
	Vector4 const SrcB2 = _m2[2];
	Vector4 const SrcB3 = _m2[3];

	Matrix4 Result;
	Result[0] = SrcA0 * SrcB0[0] + SrcA1 * SrcB0[1] + SrcA2 * SrcB0[2] + SrcA3 * SrcB0[3];
	Result[1] = SrcA0 * SrcB1[0] + SrcA1 * SrcB1[1] + SrcA2 * SrcB1[2] + SrcA3 * SrcB1[3];
	Result[2] = SrcA0 * SrcB2[0] + SrcA1 * SrcB2[1] + SrcA2 * SrcB2[2] + SrcA3 * SrcB2[3];
	Result[3] = SrcA0 * SrcB3[0] + SrcA1 * SrcB3[1] + SrcA2 * SrcB3[2] + SrcA3 * SrcB3[3];
	return Result;
}

inline Vector4 operator*(const Matrix4& _m, const Vector4& _v)
{
	Vector4 const Mov0(_v[0]);
	Vector4 const Mov1(_v[1]);
	Vector4 const Mul0 = _m[0] * Mov0;
	Vector4 const Mul1 = _m[1] * Mov1;
	Vector4 const Add0 = Mul0 + Mul1;
	Vector4 const Mov2(_v[2]);
	Vector4 const Mov3(_v[3]);
	Vector4 const Mul2 = _m[2] * Mov2;
	Vector4 const Mul3 = _m[3] * Mov3;
	Vector4 const Add1 = Mul2 + Mul3;
	Vector4 const Add2 = Add0 + Add1;
	return Add2;
}

inline Vector3 operator*(const Matrix4& _m, const Vector3& _v)
{
	Vector4 r = _m * Vector4(_v, 1.f);
	return Vector3(r.x, r.y, r.z);
}

inline Vector2 operator*(const Matrix4& _m, const Vector2& _v)
{
	Vector4 r = _m * Vector4(_v, 0.f, 1.f);
	return Vector2(r.x, r.y);
}

inline Matrix4 operator/(const Matrix4& _m, float _s)
{
	return Matrix4(
		_m[0] / _s,
		_m[1] / _s,
		_m[2] / _s,
		_m[3] / _s
	);
}

inline Matrix4 operator/(float _s, const Matrix4& _m)
{
	return Matrix4(
		_s / _m[0],
		_s / _m[1],
		_s / _m[2],
		_s / _m[3]
	);
}

inline Matrix4 operator/(const Matrix4& _m1, const Matrix4& _m2)
{
	float Coef00 = _m2[2][2] * _m2[3][3] - _m2[3][2] * _m2[2][3];
	float Coef02 = _m2[1][2] * _m2[3][3] - _m2[3][2] * _m2[1][3];
	float Coef03 = _m2[1][2] * _m2[2][3] - _m2[2][2] * _m2[1][3];

	float Coef04 = _m2[2][1] * _m2[3][3] - _m2[3][1] * _m2[2][3];
	float Coef06 = _m2[1][1] * _m2[3][3] - _m2[3][1] * _m2[1][3];
	float Coef07 = _m2[1][1] * _m2[2][3] - _m2[2][1] * _m2[1][3];

	float Coef08 = _m2[2][1] * _m2[3][2] - _m2[3][1] * _m2[2][2];
	float Coef10 = _m2[1][1] * _m2[3][2] - _m2[3][1] * _m2[1][2];
	float Coef11 = _m2[1][1] * _m2[2][2] - _m2[2][1] * _m2[1][2];

	float Coef12 = _m2[2][0] * _m2[3][3] - _m2[3][0] * _m2[2][3];
	float Coef14 = _m2[1][0] * _m2[3][3] - _m2[3][0] * _m2[1][3];
	float Coef15 = _m2[1][0] * _m2[2][3] - _m2[2][0] * _m2[1][3];

	float Coef16 = _m2[2][0] * _m2[3][2] - _m2[3][0] * _m2[2][2];
	float Coef18 = _m2[1][0] * _m2[3][2] - _m2[3][0] * _m2[1][2];
	float Coef19 = _m2[1][0] * _m2[2][2] - _m2[2][0] * _m2[1][2];

	float Coef20 = _m2[2][0] * _m2[3][1] - _m2[3][0] * _m2[2][1];
	float Coef22 = _m2[1][0] * _m2[3][1] - _m2[3][0] * _m2[1][1];
	float Coef23 = _m2[1][0] * _m2[2][1] - _m2[2][0] * _m2[1][1];

	Vector4 Fac0(Coef00, Coef00, Coef02, Coef03);
	Vector4 Fac1(Coef04, Coef04, Coef06, Coef07);
	Vector4 Fac2(Coef08, Coef08, Coef10, Coef11);
	Vector4 Fac3(Coef12, Coef12, Coef14, Coef15);
	Vector4 Fac4(Coef16, Coef16, Coef18, Coef19);
	Vector4 Fac5(Coef20, Coef20, Coef22, Coef23);

	Vector4 Vec0(_m2[1][0], _m2[0][0], _m2[0][0], _m2[0][0]);
	Vector4 Vec1(_m2[1][1], _m2[0][1], _m2[0][1], _m2[0][1]);
	Vector4 Vec2(_m2[1][2], _m2[0][2], _m2[0][2], _m2[0][2]);
	Vector4 Vec3(_m2[1][3], _m2[0][3], _m2[0][3], _m2[0][3]);

	Vector4 Inv0(Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2);
	Vector4 Inv1(Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4);
	Vector4 Inv2(Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5);
	Vector4 Inv3(Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5);

	Vector4 SignA(+1.f, -1.f, +1.f, -1.f);
	Vector4 SignB(-1.f, +1.f, -1.f, +1.f);
	Matrix4 Inverse(Inv0 * SignA, Inv1 * SignB, Inv2 * SignA, Inv3 * SignB);

	Vector4 Row0(Inverse[0][0], Inverse[1][0], Inverse[2][0], Inverse[3][0]);

	Vector4 Dot0(_m2[0] * Row0);
	float Dot1 = (Dot0.x + Dot0.y) + (Dot0.z + Dot0.w);

	float OneOverDeterminant = 1.f / Dot1;
	Inverse *= OneOverDeterminant;

	return _m1 * Inverse;
}

// -- Boolean operators --
inline bool operator==(const Matrix4& _m1, const Matrix4& _m2)
{
	return (_m1[0] == _m2[0]) && (_m1[1] == _m2[1]) && (_m1[2] == _m2[2]) && (_m1[3] == _m2[3]);
}

inline bool operator!=(const Matrix4& _m1, const Matrix4& _m2)
{
	return (_m1[0] != _m2[0]) || (_m1[1] != _m2[1]) || (_m1[2] != _m2[2]) || (_m1[3] != _m2[3]);
}

} // namespace yae
