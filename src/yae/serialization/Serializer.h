#pragma once

#include <yae/types.h>
#include <yae/string.h>

namespace yae {

class Allocator;

enum class SerializationMode : u8
{
	NONE = 0,
	READ,
	WRITE
};

class YAELIB_API Serializer 
{
public:
	Serializer(Allocator* _allocator = nullptr);
	virtual ~Serializer();

	SerializationMode getMode() const;

	virtual void beginWrite();
	virtual void endWrite();

	virtual void beginRead(void* _data, u32 _dataSize);
	virtual void endRead();

	virtual bool serialize(bool& _value, const char* _key = nullptr) = 0;
	virtual bool serialize(u8& _value, const char* _key = nullptr) = 0;
	virtual bool serialize(u16& _value, const char* _key = nullptr) = 0;
	virtual bool serialize(u32& _value, const char* _key = nullptr) = 0;
	virtual bool serialize(u64& _value, const char* _key = nullptr) = 0;
	virtual bool serialize(i8& _value, const char* _key = nullptr) = 0;
	virtual bool serialize(i16& _value, const char* _key = nullptr) = 0;
	virtual bool serialize(i32& _value, const char* _key = nullptr) = 0;
	virtual bool serialize(i64& _value, const char* _key = nullptr) = 0;
	virtual bool serialize(float& _value, const char* _key = nullptr) = 0;
	virtual bool serialize(double& _value, const char* _key = nullptr) = 0;

	virtual bool beginSerializeArray(u32& _size, const char* _key = nullptr) = 0;
	virtual bool endSerializeArray() = 0;

	virtual bool beginSerializeObject(const char* _key = nullptr) = 0;
	virtual bool endSerializeObject() = 0;

	virtual void* getData() const = 0;
	virtual u32 getDataSize() const = 0;

	const char* getLastError() const;

//private:
	String m_lastError;
	SerializationMode m_mode = SerializationMode::NONE;
	Allocator* m_allocator = nullptr;
};

} // namespace yae
