#include "BinarySerializer.h"

#include <core/memory.h>
#include <core/math.h>
#include <core/string.h>

/*
Memory layout:

dataBlock ~b
 dataSize 32b
 data ~b

ArrayDataBlock
 dataSize 32b
 data ~b
  arrayElementCount 32b
  dataBlock []~b

object data block
 dataSize 32b
 data ~b
  objectDataSize 32b
  objectData ~b
  addressMapElementCount 32b
  addressMap []64b
  	keyhash 32b
  	offset 32b
*/

namespace yae {

const u32 SIZE_FIELD_SIZE = sizeof(u32); // Size of a size field in the binary buffer
const u32 INDEX_PAIR_SIZE = sizeof(u32) * 2;

BinarySerializer::BinarySerializer(Allocator* _allocator)
	: Serializer(_allocator)
	, m_bufferStack(_allocator)
{

}

BinarySerializer::~BinarySerializer()
{
	m_allocator->deallocate(m_writeData);
}

void BinarySerializer::beginWrite()
{
	YAE_ASSERT(m_bufferStack.size() == 0);
	m_allocator->deallocate(m_writeData);
	m_writeData = nullptr;
	m_writeDataSize = 0;
	Serializer::beginWrite();

	Buffer buffer;
	buffer.type = BufferType::PLAIN;
	buffer.data = nullptr;
	buffer.dataSize = 0;
	buffer.cursor = 0;
	buffer.maxCursor = 0;
	m_bufferStack.push_back(buffer);
}

void BinarySerializer::endWrite()
{
	Serializer::endWrite();

	YAE_ASSERT(m_bufferStack.size() == 1);

	Buffer& buffer = _getTopBuffer();

	YAE_ASSERT(m_writeData == nullptr);
	m_writeDataSize = buffer.dataSize;
	m_writeData = m_allocator->allocate(m_writeDataSize);
	memcpy(m_writeData, buffer.data, m_writeDataSize);
	m_allocator->deallocate(buffer.data);
	m_bufferStack.pop_back();
}

void* BinarySerializer::getWriteData() const
{
	YAE_ASSERT_MSG(getMode() == SerializationMode::NONE, "WriteData can't be queried during a serialization. This call should go outside of the begin/end block.");
	return m_writeData;
}

u32 BinarySerializer::getWriteDataSize() const
{
	YAE_ASSERT_MSG(getMode() == SerializationMode::NONE, "WriteData can't be queried during a serialization. This call should go outside of the begin/end block.");
	return m_writeDataSize;
}

void BinarySerializer::setReadData(void* _data, u32 _dataSize)
{
	YAE_ASSERT_MSG(getMode() == SerializationMode::NONE, "ReadData can't be set during a serialization. This call should go outside of the begin/end block.");
	m_readData = _data;
	m_readDataSize = _dataSize;
}

void BinarySerializer::beginRead()
{
	YAE_ASSERT(m_bufferStack.size() == 0);
	YAE_ASSERT(m_readDataSize = 0 || m_readData != nullptr);

	Serializer::beginRead();

	Buffer buffer;
	buffer.type = BufferType::PLAIN;
	buffer.data = (u8*)m_readData;
	buffer.dataSize = m_readDataSize;
	buffer.cursor = 0;
	buffer.maxCursor = 0;
	m_bufferStack.push_back(buffer);
}

void BinarySerializer::endRead()
{
	m_bufferStack.pop_back();
	YAE_ASSERT(m_bufferStack.size() == 0);
	m_readData = nullptr;
	m_readDataSize = 0;

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

bool BinarySerializer::serialize(u16& _value, const char* _id)
{
	return _serializeData(&_value, sizeof(_value), _id);
}

bool BinarySerializer::serialize(u32& _value, const char* _id)
{
	return _serializeData(&_value, sizeof(_value), _id);
}

bool BinarySerializer::serialize(u64& _value, const char* _id)
{
	return _serializeData(&_value, sizeof(_value), _id);
}

bool BinarySerializer::serialize(i8& _value, const char* _id)
{
	return _serializeData(&_value, sizeof(_value), _id);
}

bool BinarySerializer::serialize(i16& _value, const char* _id)
{
	return _serializeData(&_value, sizeof(_value), _id);
}

bool BinarySerializer::serialize(i32& _value, const char* _id)
{
	return _serializeData(&_value, sizeof(_value), _id);
}

bool BinarySerializer::serialize(i64& _value, const char* _id)
{
	return _serializeData(&_value, sizeof(_value), _id);
}

bool BinarySerializer::serialize(float& _value, const char* _id)
{
	return _serializeData(&_value, sizeof(_value), _id);
}

bool BinarySerializer::serialize(double& _value, const char* _id)
{
	return _serializeData(&_value, sizeof(_value), _id);
}

bool BinarySerializer::serialize(String& _value, const char* _id)
{
	YAE_ASSERT(m_mode != SerializationMode::NONE);

	_processId(_id);

	u32 stringSize = _value.size();
	if (!serialize(stringSize))
		return false;

	if (m_mode == SerializationMode::READ)
		_value.resize(stringSize);

	return _serializeDataRaw(_value.data(), sizeof(char) * stringSize);
}

bool BinarySerializer::beginSerializeArray(u32& _size, const char* _id)
{
	YAE_ASSERT(m_mode != SerializationMode::NONE);

	_processId(_id);

	Buffer& parentBuffer = _getTopBuffer();

	Buffer newBuffer(m_allocator);
	newBuffer.type = BufferType::ARRAY;
	switch(m_mode)
	{
		case SerializationMode::READ:
		{
			newBuffer.data = parentBuffer.data + parentBuffer.cursor;
			newBuffer.dataSize = *(u32*)(newBuffer.data);
			newBuffer.array.elementCount = *(u32*)(newBuffer.data + SIZE_FIELD_SIZE);
			_size = newBuffer.array.elementCount;
			_setBufferCursor(newBuffer, SIZE_FIELD_SIZE * 2); // dataSize + array elementCount
		}
		break;

		case SerializationMode::WRITE:
		{
			newBuffer.data = nullptr;
			newBuffer.dataSize = 0;
			_setBufferCursor(newBuffer, SIZE_FIELD_SIZE * 2); // reserve space for dataSize + array elementCount
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

	_setBufferCursor(parentBuffer, parentBuffer.cursor + topBuffer.cursor);
	m_bufferStack.pop_back();

	return true;
}

bool BinarySerializer::beginSerializeObject(const char* _id)
{
	YAE_ASSERT(m_mode != SerializationMode::NONE);

	_processId(_id);

	Buffer& parentBuffer = _getTopBuffer();

	Buffer newBuffer(m_allocator);
	newBuffer.type = BufferType::OBJECT;
	switch(m_mode)
	{
		case SerializationMode::READ:
		{
			newBuffer.data = parentBuffer.data + parentBuffer.cursor;
			newBuffer.dataSize = *(u32*)(newBuffer.data);
			u32 objectDataSize = *(u32*)(newBuffer.data + SIZE_FIELD_SIZE);
			u8* indexAddress = newBuffer.data + (SIZE_FIELD_SIZE * 2) + objectDataSize;
			u32 indexSize = *(u32*)indexAddress;
			newBuffer.object.index.clear();
			for (u32 i = 0; i < indexSize; ++i)
			{
				u8* address = indexAddress + SIZE_FIELD_SIZE + (i * INDEX_PAIR_SIZE);
				StringHash key = StringHash(*(u32*)address);
				u32 offset = *(u32*)(address + SIZE_FIELD_SIZE);
				newBuffer.object.index.set(key, offset);
			}
			_setBufferCursor(newBuffer, SIZE_FIELD_SIZE * 2); // dataSize + objectDataSize
		}
		break;

		case SerializationMode::WRITE:
		{
			newBuffer.data = nullptr;
			newBuffer.dataSize = 0;
			_setBufferCursor(newBuffer, SIZE_FIELD_SIZE * 2); // reserve space for dataSize + objectDataSize
			newBuffer.object.index.clear();
			_growBuffer(newBuffer, newBuffer.cursor);
		}
		break;
	}
	m_bufferStack.push_back(newBuffer);

	return true;
}

bool BinarySerializer::endSerializeObject()
{
	YAE_ASSERT(m_mode != SerializationMode::NONE);
	YAE_ASSERT(m_bufferStack.size() >= 2);

	Buffer& topBuffer = _getTopBuffer();
	YAE_ASSERT(topBuffer.type == BufferType::OBJECT);
	Buffer& parentBuffer = m_bufferStack[m_bufferStack.size() - 2];

	switch(m_mode)
	{
		case SerializationMode::READ:
		{

		}
		break;

		case SerializationMode::WRITE:
		{
			// put cursor back at the end of the array
			_setBufferCursor(topBuffer, topBuffer.maxCursor);

			// dataObjectSize (size do not include dataObjectSize field)
			*(u32*)(topBuffer.data + SIZE_FIELD_SIZE) = topBuffer.cursor - (SIZE_FIELD_SIZE * 2);

			// Write index
			u32 addressMapElementCount = topBuffer.object.index.size();
			_serializeDataRaw(&addressMapElementCount, sizeof(addressMapElementCount));
			for (const auto& pair : topBuffer.object.index)
			{
				u32 key = pair.key;
				u32 value = pair.value;
				_serializeDataRaw(&key, sizeof(key));
				_serializeDataRaw(&value, sizeof(value));
			}

			// Write data size
			*(u32*)(topBuffer.data) = topBuffer.cursor;

			// Write top buffer into parent buffer
			_growBuffer(parentBuffer, topBuffer.cursor);
			memcpy(parentBuffer.data + parentBuffer.cursor, topBuffer.data, topBuffer.cursor);

			// Free memory
			m_allocator->deallocate(topBuffer.data);
			topBuffer.data = nullptr;
		}
		break;
	}

	_setBufferCursor(parentBuffer, parentBuffer.cursor + topBuffer.cursor);
	m_bufferStack.pop_back();

	return true;
}

bool BinarySerializer::_serializeData(void* _data, u32 _size, const char* _id)
{
	YAE_ASSERT(m_mode != SerializationMode::NONE);

	_processId(_id);

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

	_setBufferCursor(buffer, buffer.cursor + _size + SIZE_FIELD_SIZE);
	return true;
}

bool BinarySerializer::_serializeDataRaw(void* _data, u32 _size, const char* _id)
{
	YAE_ASSERT(m_mode != SerializationMode::NONE);

	_processId(_id);

	Buffer& buffer = _getTopBuffer();

	switch(m_mode)
	{
		case SerializationMode::READ:
		{
			memcpy(_data, buffer.data + buffer.cursor, _size);
		}
		break;

		case SerializationMode::WRITE:
		{
			_growBuffer(buffer, _size);
			memcpy(buffer.data + buffer.cursor, _data, _size);
		}
		break;
	}

	_setBufferCursor(buffer, buffer.cursor + _size);
	return true;
}

bool BinarySerializer::_processId(const char* _id)
{
	if (_id == nullptr)
		return true;

	Buffer& buffer = _getTopBuffer();

	YAE_ASSERT_MSG(buffer.type == BufferType::OBJECT, "Cannot serialize fields with id outside of a begin/endSerializeObject block.");

	u32 key = StringHash(_id);

	switch(m_mode)
	{
		case SerializationMode::READ:
		{
			// Move cursor to the offset corresponding the id
			u32* offsetPointer = buffer.object.index.get(key);
			if (offsetPointer == nullptr)
			{
				m_lastError = string::format("Can't find id \"%s\".", _id);
				YAE_ASSERT(false);
				return false;
			}

			_setBufferCursor(buffer, *offsetPointer);
		}
		break;

		case SerializationMode::WRITE:
		{
			// Store current offset with this id
			u32 offset = buffer.cursor;
			u32* offsetPointer = buffer.object.index.get(key);
			if (offsetPointer != nullptr)
			{
				YAE_ASSERT_MSGF(false, "Serializing the id \"%s\" twice. Should we support this ?", _id);
				*offsetPointer = offset;
				_setBufferCursor(buffer, *offsetPointer);
			}
			else
			{
				buffer.object.index.set(key, offset);
				_setBufferCursor(buffer, buffer.maxCursor);
			}
		}
		break;
	}

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

void BinarySerializer::_setBufferCursor(Buffer& _buffer, u32 _cursor)
{
	YAE_ASSERT(m_mode != SerializationMode::NONE);

	_buffer.cursor = _cursor;
	_buffer.maxCursor = math::max(_cursor, _buffer.maxCursor);
}

BinarySerializer::Buffer& BinarySerializer::_getTopBuffer()
{
	YAE_ASSERT(m_bufferStack.size() > 0);
	return m_bufferStack.back();
}

} // namespace yae
