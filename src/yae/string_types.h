#pragma once

#include <yae/types.h>

#include <cstdio>

namespace yae {

// @TODO: use explicit types of sizes (u32?)

typedef u8 StringFlags;
enum StringFlags_
{
	StringFlags_AppendedBuffer = 1 << 0,
};

class YAE_API String
{
public:
	static const size_t INVALID_POS;

	String(Allocator* _allocator = nullptr);
	String(const char* _str, Allocator* _allocator = nullptr);
	String(const String& _str, Allocator* _allocator = nullptr);
	~String();

	String(String&& _str);

	size_t size() const { return m_length; }
	size_t capacity() const { return m_bufferSize; }
	const char* c_str() const;
	const char* data() const;
	char* data();
	Allocator* allocator() const { return m_allocator; }

	void reserve(size_t _size);
	void resize(size_t _size, char _c = ' ');
	void clear();
	void shrink();

	size_t find(const char* _str, size_t _startPosition = 0) const;
	String& replace(const char* _toReplace, const char* _replacement);
	String& replace(size_t _position, size_t _count, const char* _replacement);
	String slice(size_t _startPosition, size_t _count) const;

	String& operator=(const char* _str);
	String& operator=(const String& _str);
	String operator+(char _char) const;
	String operator+(const char* _str) const;
	String operator+(const String& _str) const;
	String& operator+=(char _char);
	String& operator+=(const char* _str);
	String& operator+=(const String& _str);
	char& operator[](size_t _pos);
	const char& operator[](size_t _pos) const;
	bool operator==(const char* _str) const;
	bool operator==(const String& _str) const;
	bool operator!=(const char* _str) const;
	bool operator!=(const String& _str) const;

protected:
	String(Allocator* _allocator, size_t _appendedBufferCapacity);
	String(const char* _str, Allocator* _allocator, size_t _appendedBufferCapacity);

private:
	char* m_buffer = nullptr;
	size_t m_bufferSize = 0;
	size_t m_length = 0;
	Allocator* m_allocator = nullptr;
	u8 m_flags = 0;
};

YAE_API String operator+(const char* _lhs, const String& _rhs);


class YAE_API MallocString : public String
{
public:
	MallocString();
	MallocString(const char* _str);
	MallocString(const String& _str);
};

template <size_t INLINE_SIZE>
class InlineString : public String
{
public:
	InlineString(Allocator* _allocator = nullptr)
		:String(_allocator, INLINE_SIZE)
	{
		YAE_ASSERT(data() == m_buffer);
	}

	InlineString(const char* _str, Allocator* _allocator = nullptr)
		:String(_str, _allocator, INLINE_SIZE)
	{
		YAE_ASSERT(capacity() > INLINE_SIZE || data() == m_buffer);
	}

	InlineString(const String& _str, Allocator* _allocator = nullptr)
		:String(_str.c_str(), _allocator, INLINE_SIZE)
	{
		YAE_ASSERT(capacity() > INLINE_SIZE || data() == m_buffer);
	}

private:
	char m_buffer[INLINE_SIZE];
};

typedef InlineString<32> String32;
typedef InlineString<64> String64;
typedef InlineString<128> String128;
typedef InlineString<256> String256;
typedef InlineString<512> String512;

} // namespace yae
