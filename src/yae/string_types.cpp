#include "string_types.h"

#include <yae/program.h>
#include <yae/memory.h>

#include <cstring>
#include <algorithm>

namespace yae {

const char* EMPTY_STRING = "";
const size_t String::INVALID_POS = size_t(-1);

String::String(Allocator* _allocator)
	: m_buffer(nullptr)
	, m_allocator(_allocator)
{
	if (m_allocator == nullptr)
	{
		m_allocator = &defaultAllocator();
	}
	YAE_ASSERT(m_allocator);
}

String::String(const char* _str, Allocator* _allocator)
	:String(_allocator)
{
	size_t len = strlen(_str);
	if (len == 0)
		return;

	reserve(len);
	memcpy(data(), _str, len + 1);
	m_length = len;
}


// @NOTE: not sure if replicating the allocator is the right thing to do, but it does solve issues when there are intermediate copies made. If it does the logic should be extended to containers
String::String(const String& _str, Allocator* _allocator)
	: String(_str.c_str(), _allocator != nullptr ? _allocator : _str.m_allocator)
{

}

String::~String()
{
	if (m_buffer != nullptr)
	{
		m_allocator->deallocate(m_buffer);	
		m_buffer = nullptr;	
	}
}

String::String(String&& _str)
{
	m_buffer = _str.m_buffer;
	m_bufferSize = _str.m_bufferSize;
	m_length = _str.m_length;
	m_allocator = _str.m_allocator;
	m_flags = _str.m_flags;
	
	_str.m_buffer = nullptr;
}

const char* String::c_str() const
{
	return data() != nullptr ? data() : EMPTY_STRING;
}

const char* String::data() const
{
	if (m_flags & StringFlags_AppendedBuffer)
	{
		return (char*)this + sizeof(String);
	}
	return m_buffer;
}

char* String::data()
{
	return const_cast<char*>(const_cast<const String*>(this)->data());
}


void String::reserve(size_t _size)
{
	size_t sizeRequired = _size + 1;
	if (sizeRequired > m_bufferSize)
	{
		m_buffer = (char*)m_allocator->reallocate(m_buffer, sizeRequired);
		m_bufferSize = sizeRequired;
		m_flags &= ~StringFlags_AppendedBuffer;
		YAE_ASSERT_MSG(m_buffer != nullptr, "Allocation failed");
	}
}



void String::resize(size_t _size, char _c)
{
	if (_size < m_length)
	{
		m_length = _size;
		data()[m_length] = 0;
	}
	else if (_size > m_length)
	{
		reserve(_size);
		memset(data() + m_length, _c, _size - m_length);
		m_length = _size;
		data()[m_length] = 0;
	}
}



void String::clear()
{
	if (data() != nullptr)
	{
		m_length = 0;
		data()[m_length] = 0;
	}
}


void String::shrink()
{
	if (m_flags & StringFlags_AppendedBuffer)
		return; // Do nothing if the buffer is inlined in the struct

	// @WARNING: NOT TESTED
	if (m_length == 0 && m_buffer != nullptr)
	{
		m_allocator->deallocate(m_buffer);
		m_buffer = nullptr;
		m_bufferSize = 0;
	}
	else if (m_length > 0)
	{
		size_t newCapacity = m_length + 1;
		m_buffer = (char*)m_allocator->reallocate(m_buffer, newCapacity);
		m_bufferSize = newCapacity;
	}
}


size_t String::find(const char* _str, size_t _startPosition) const
{
	if (data() == nullptr || _str == nullptr)
		return INVALID_POS;

	if (_startPosition >= m_length)
		return INVALID_POS;

	const char* ptr = strstr(data() + _startPosition, _str);
	if (ptr == nullptr)
		return INVALID_POS;

	return size_t(ptr - data());
}



String& String::replace(const char* _toReplace, const char* _replacement)
{
	size_t toReplaceLen = strlen(_toReplace);
	size_t replacementLen = strlen(_replacement);
	size_t pos = find(_toReplace);
	while (pos != INVALID_POS)
	{
		replace(pos, toReplaceLen, _replacement);
		pos = find(_toReplace, pos + replacementLen);
	}
	return *this;
}



String String::slice(size_t _startPosition, size_t _count) const
{
	String str = String(&scratchAllocator());

	if (_startPosition > m_length)
		return str;

	_count = std::min(_count, m_length - _startPosition);
	str.resize(_count);
	memcpy(str.data(), data() + _startPosition, _count);
	return str;
}



String& String::replace(size_t _position, size_t _count, const char* _replacement)
{
	if (m_length == 0)
		return *this;

	size_t baseLength = m_length;
	size_t count = std::min(baseLength - _position, _count);
	size_t replacementLen = strlen(_replacement);

	if (_count < replacementLen) // if replacement string is longer, resize before
	{
		resize(baseLength + replacementLen - count);
	}

	char* bufferPosition = data() + _position;
	memcpy(bufferPosition + replacementLen, bufferPosition + count, baseLength - (_position + count));
	memcpy(bufferPosition, _replacement, replacementLen);

	if (_count > replacementLen) // if replacement string is shorted, resize after
	{
		resize(baseLength + replacementLen - count);
	}

	return *this;
}

String& String::operator=(const char* _str)
{
	size_t length = strlen(_str);
	reserve(length);
	memcpy(data(), _str, length + 1);
	m_length = length;
	return *this;
}

String& String::operator=(const String& _str)
{
	reserve(_str.m_length);
	memcpy(data(), _str.c_str(), _str.m_length + 1);
	m_length = _str.m_length;
	return *this;
}


String String::operator+(char _char) const
{
	String result = *this;
	result += _char;
	return result;
}



String String::operator+(const char* _str) const
{
	String result = *this;
	result += _str;
	return result;
}


String String::operator+(const String& _str) const
{
	String result = *this;
	result += _str;
	return result;
}


String& String::operator+=(char _char)
{
	reserve(m_length + 1);
	data()[m_length] = _char;
	m_length = m_length + 1;
	data()[m_length] = 0;
	return *this;
}


String& String::operator+=(const char* _str)
{
	size_t length = strlen(_str);
	size_t newLength = m_length + length;
	reserve(newLength);
	memcpy(data() + m_length, _str, length);
	data()[newLength] = 0;
	m_length = newLength;
	return *this;
}


String& String::operator+=(const String& _str)
{
	size_t length = _str.m_length;
	size_t newLength = m_length + length;
	reserve(newLength);
	memcpy(data() + m_length, _str.data(), length);
	data()[newLength] = 0;
	m_length = newLength;
	return *this;
}


char& String::operator[](size_t _pos)
{
	YAE_ASSERT(_pos >= 0);
	YAE_ASSERT(_pos < m_length);
	return data()[_pos];
}


const char& String::operator[](size_t _pos) const
{
	YAE_ASSERT(_pos >= 0);
	YAE_ASSERT(_pos < m_length);
	return data()[_pos];
}

bool String::operator==(const char* _str) const
{
	YAE_ASSERT(_str != nullptr);
	size_t strLength = strlen(_str);
	return (m_length == strLength) && (m_length == 0 || strcmp(_str, c_str()) == 0);
}

bool String::operator==(const String& _str) const
{
	return (m_length == _str.m_length) && (m_length == 0 || strcmp(_str.c_str(), c_str()) == 0);
}

bool String::operator!=(const char* _str) const
{
	return !(*this == _str);
}

bool String::operator!=(const String& _str) const
{
	return !(*this == _str);
}

String::String(Allocator* _allocator, size_t _appendedBufferCapacity)
	: m_bufferSize(_appendedBufferCapacity)
	, m_allocator(_allocator)
	, m_flags(StringFlags_AppendedBuffer)
{
}

String::String(const char* _str, Allocator* _allocator, size_t _appendedBufferCapacity)
	: m_bufferSize(_appendedBufferCapacity)
	, m_allocator(_allocator)
	, m_flags(StringFlags_AppendedBuffer)
{
	*this = _str;
}

String operator+(const char* _lhs, const String& _rhs)
{
	String result(_rhs.allocator());
	result += _lhs;
	result += _rhs;
	return result;
}

MallocString::MallocString()
	: String(&mallocAllocator())
{
}

MallocString::MallocString(const char* _str)
	: String(_str, &mallocAllocator())
{
}

MallocString::MallocString(const String& _str)
	: String(_str, &mallocAllocator())
{
}

} // namespace yae
