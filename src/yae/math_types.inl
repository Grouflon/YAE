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
	return Vector4(
		_v.x / _s,
		_v.y / _s,
		_v.z / _s,
		_v.w / _s
	);

}

inline Vector4 operator/(float _s, const Vector4& _v)
{
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
	this->m[0] *= _m[0];
	this->m[1] *= _m[1];
	this->m[2] *= _m[2];
	return *this;
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
	this->m[0] /= _m[0];
	this->m[1] /= _m[1];
	this->m[2] /= _m[2];
	return *this;
}


} // namespace yae
