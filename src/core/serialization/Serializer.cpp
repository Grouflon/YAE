#include "Serializer.h"

namespace yae {

Serializer::Serializer(Allocator* _allocator)
	: m_lastError(_allocator)
	, m_allocator(_allocator)
{
	if (m_allocator == nullptr)
	{
		m_allocator = &defaultAllocator();
	}
	YAE_ASSERT(m_allocator);
}

Serializer::~Serializer()
{
	YAE_ASSERT(m_mode == SerializationMode::NONE);
}

SerializationMode Serializer::getMode() const
{
	return m_mode;
}

bool Serializer::isReading() const
{
	return m_mode == SerializationMode::READ;
}

bool Serializer::isWriting() const
{
	return m_mode == SerializationMode::WRITE;
}

void Serializer::beginWrite()
{
	YAE_ASSERT(m_mode == SerializationMode::NONE);

	m_mode = SerializationMode::WRITE;
}

void Serializer::endWrite()
{
	YAE_ASSERT(m_mode == SerializationMode::WRITE);

	m_mode = SerializationMode::NONE;
}

void Serializer::beginRead()
{
	YAE_ASSERT(m_mode == SerializationMode::NONE);

	m_mode = SerializationMode::READ;
}

void Serializer::endRead()
{
	YAE_ASSERT(m_mode == SerializationMode::READ);

	m_mode = SerializationMode::NONE;
}

const char* Serializer::getLastError() const
{
	return m_lastError.c_str();	
}

} // namespace yae
