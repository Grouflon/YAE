#include "yae_string.h"

#include <cstring>
#include <string>
#include <algorithm>

#include <memory.h>
#include <context.h>

namespace yae {

const size_t String::INVALID_POS = size_t(-1);

String::String(Allocator* _allocator)
	: m_allocator(_allocator)
{
	if (m_allocator == nullptr)
	{
		m_allocator = context().defaultAllocator;
	}
	YAE_ASSERT(m_allocator);
}



String::String(const char* _str, Allocator* _allocator)
	:String(_allocator)
{
	size_t len = strlen(_str);
	reserve(len);
	memcpy(m_buffer, _str, len + 1);
	m_length = len;
}



String::String(const String& _str, Allocator* _allocator)
	: String(_str.c_str(), _allocator)
{

}



String::~String()
{
	m_allocator->deallocate(m_buffer);
}



String::String(String&& _str)
{
	m_buffer = _str.m_buffer;
	m_bufferSize = _str.m_bufferSize;
	m_length = _str.m_length;
	m_allocator = _str.m_allocator;
	
	_str.m_buffer = nullptr;
}



void String::reserve(size_t _size)
{
	size_t sizeRequired = _size + 1;
	if (sizeRequired > m_bufferSize)
	{
		void* buffer = m_allocator->allocate(sizeRequired);
		memcpy(buffer, m_buffer, m_bufferSize);
		m_allocator->deallocate(m_buffer);
		m_buffer = (char*)buffer;
		m_bufferSize = sizeRequired;
		YAE_ASSERT_MSG(m_buffer != nullptr, "Allocation failed");
	}
}



void String::resize(size_t _size, char _c)
{
	if (_size < m_length)
	{
		m_length = _size;
		m_buffer[m_length] = 0;
	}
	else if (_size > m_length)
	{
		reserve(_size);
		memset(m_buffer + m_length, _c, _size - m_length);
		m_length = _size;
		m_buffer[m_length] = 0;
	}
}



void String::clear()
{
	if (m_buffer != nullptr)
	{
		m_length = 0;
		m_buffer[m_length] = 0;
	}
}



size_t String::find(const char* _str, size_t _startPosition) const
{
	if (m_buffer == nullptr || _str == nullptr)
		return INVALID_POS;

	if (_startPosition >= m_length)
		return INVALID_POS;

	char* ptr = strstr(m_buffer + _startPosition, _str);
	if (ptr == nullptr)
		return INVALID_POS;

	return size_t(ptr - m_buffer);
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



String& String::replace(size_t _position, size_t _count, const char* _replacement)
{
	size_t baseLength = m_length;
	size_t count = std::min(baseLength - _position, _count);
	size_t replacementLen = strlen(_replacement);
	resize(baseLength + replacementLen - count);
	char* bufferPosition = m_buffer + _position;
	memcpy(bufferPosition + replacementLen, bufferPosition + count, baseLength - (_position + count));
	memcpy(bufferPosition, _replacement, replacementLen);

	return *this;
}

String& String::operator=(const char* _str)
{
	size_t length = strlen(_str);
	reserve(length);
	memcpy(m_buffer, _str, length + 1);
	m_length = length;
	return *this;
}

String& String::operator=(const String& _str)
{
	reserve(_str.m_length);
	memcpy(m_buffer, _str.m_buffer, _str.m_length + 1);
	m_length = _str.m_length;
	return *this;
}


MallocAllocator s_mallocAllocator;
MallocString::MallocString() : String(&s_mallocAllocator) {}
/*MallocString::MallocString(const char* _str) : String(_str, &s_mallocAllocator) {}
MallocString::MallocString(const String& _str) : String(_str, &s_mallocAllocator) {}*/


namespace string {



} // namespace string

} // namespace yae
