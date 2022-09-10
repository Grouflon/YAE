#include "Serializer.h"

#include <yae/memory.h>

namespace yae {

const u32 SIZE_FIELD_SIZE = sizeof(u32); // Size of a size field in the binary buffer

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

void Serializer::beginRead(void* _data, u32 _dataSize)
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

BinarySerializer::BinarySerializer(Allocator* _allocator)
	: Serializer(_allocator)
	, m_bufferStack(_allocator)
{

}

BinarySerializer::~BinarySerializer()
{
}

void BinarySerializer::beginWrite()
{
	YAE_ASSERT(m_bufferStack.size() == 0);
	Serializer::beginWrite();

	Buffer buffer;
	buffer.type = BufferType::PLAIN;
	buffer.data = nullptr;
	buffer.dataSize = 0;
	buffer.cursor = 0;
	m_bufferStack.push_back(buffer);
}

void BinarySerializer::endWrite()
{
	Serializer::endWrite();

	YAE_ASSERT(m_bufferStack.size() == 1);

	Buffer& buffer = _getTopBuffer();
	m_allocator->deallocate(buffer.data);
	m_bufferStack.pop_back();
}

void BinarySerializer::beginRead(void* _data, u32 _dataSize)
{
	YAE_ASSERT(m_bufferStack.size() == 0);

	Serializer::beginRead(_data, _dataSize);

	Buffer buffer;
	buffer.type = BufferType::PLAIN;
	buffer.data = (u8*)_data;
	buffer.dataSize = _dataSize;
	buffer.cursor = 0;
	m_bufferStack.push_back(buffer);
}

void BinarySerializer::endRead()
{
	m_bufferStack.pop_back();
	YAE_ASSERT(m_bufferStack.size() == 0);

	Serializer::endRead();
}

bool BinarySerializer::serialize(bool& _value, const char* _id)
{
	return _serializeData(&_value, sizeof(_value), _id);
}

bool BinarySerializer::serialize(u8& _value, const char* _id)
{
	return _serializeData(&_value, sizeof(_value), _id);
}

bool BinarySerializer::serialize(u32& _value, const char* _id)
{
	return _serializeData(&_value, sizeof(_value), _id);
}

bool BinarySerializer::serialize(float& _value, const char* _id)
{
	return _serializeData(&_value, sizeof(_value), _id);
}

bool BinarySerializer::beginSerializeArray(u32& _size, const char* _id)
{
	YAE_ASSERT(m_mode != SerializationMode::NONE);

	Buffer& parentBuffer = _getTopBuffer();

	Buffer newBuffer;
	newBuffer.type = BufferType::ARRAY;
	switch(m_mode)
	{
		case SerializationMode::READ:
		{
			newBuffer.data = parentBuffer.data + parentBuffer.cursor;
			newBuffer.dataSize = *(u32*)(newBuffer.data);
			newBuffer.array.elementCount = *(u32*)(newBuffer.data + SIZE_FIELD_SIZE);
			_size = newBuffer.array.elementCount;
			newBuffer.cursor = SIZE_FIELD_SIZE * 2; // dataSize + array elementCount
		}
		break;

		case SerializationMode::WRITE:
		{
			newBuffer.data = nullptr;
			newBuffer.dataSize = 0;
			newBuffer.cursor = SIZE_FIELD_SIZE * 2; // reserve space for dataSize + array elementCount
			newBuffer.array.elementCount = _size;
			_growBuffer(newBuffer, newBuffer.cursor);
		}
		break;
	}
	m_bufferStack.push_back(newBuffer);

	return true;
}

bool BinarySerializer::endSerializeArray()
{
	YAE_ASSERT(m_mode != SerializationMode::NONE);
	YAE_ASSERT(m_bufferStack.size() >= 2);

	Buffer& topBuffer = _getTopBuffer();
	YAE_ASSERT(topBuffer.type == BufferType::ARRAY);
	Buffer& parentBuffer = m_bufferStack[m_bufferStack.size() - 2];

	switch(m_mode)
	{
		case SerializationMode::READ:
		{
		}
		break;

		case SerializationMode::WRITE:
		{
			// Write size & element count
			*(u32*)(topBuffer.data) = topBuffer.cursor;
			*(u32*)(topBuffer.data + SIZE_FIELD_SIZE) = topBuffer.array.elementCount;

			// Write top buffer into parent buffer
			_growBuffer(parentBuffer, topBuffer.cursor);
			memcpy(parentBuffer.data + parentBuffer.cursor, topBuffer.data, topBuffer.cursor);

			// Free memory
			m_allocator->deallocate(topBuffer.data);
			topBuffer.data = nullptr;
		}
		break;
	}

	parentBuffer.cursor += topBuffer.cursor;
	m_bufferStack.pop_back();

	return true;
}

void* BinarySerializer::getData() const
{
	YAE_ASSERT(m_bufferStack.size() > 0);
	return m_bufferStack[0].data;
}

u32 BinarySerializer::getDataSize() const
{
	YAE_ASSERT(m_bufferStack.size() > 0);
	return m_bufferStack[0].dataSize;
}

bool BinarySerializer::_serializeData(void* _data, u32 _size, const char* _id)
{
	YAE_ASSERT(m_mode != SerializationMode::NONE);

	Buffer& buffer = _getTopBuffer();

	switch(m_mode)
	{
		case SerializationMode::READ:
		{
			u32 size = *(u32*)(buffer.data + buffer.cursor);
			if (size != _size)
			{
				m_lastError = string::format("Trying to read %d bytes, but size is %d bytes", _size, size);
				// @NOTE: let's keep this assert while this is in developement
				YAE_ASSERT(false);
				return false;
			}
			memcpy(_data, buffer.data + buffer.cursor + SIZE_FIELD_SIZE, size);
		}
		break;

		case SerializationMode::WRITE:
		{
			_growBuffer(buffer, _size + SIZE_FIELD_SIZE);
			memcpy(buffer.data + buffer.cursor, &_size, SIZE_FIELD_SIZE);
			memcpy(buffer.data + buffer.cursor + SIZE_FIELD_SIZE, _data, _size);
		}
		break;
	}

	buffer.cursor += _size + SIZE_FIELD_SIZE;
	return true;
}

void BinarySerializer::_growBuffer(Buffer& _buffer, u32 _addedSize)
{
	YAE_ASSERT(m_mode == SerializationMode::WRITE);

	bool doGrow = false;
	while ((_buffer.cursor + _addedSize) > _buffer.dataSize)
	{
		_buffer.dataSize = _buffer.dataSize * 2 + 8;
		doGrow = true;
	}
	if (doGrow)
	{
		_buffer.data = (u8*)m_allocator->reallocate(_buffer.data, _buffer.dataSize);
	}
}

BinarySerializer::Buffer& BinarySerializer::_getTopBuffer()
{
	YAE_ASSERT(m_bufferStack.size() > 0);
	return m_bufferStack.back();
}


} // namespace yae
