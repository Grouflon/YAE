#pragma once

#include <yae/types.h>
#include <yae/string.h>

#include <yae/containers/Array.h>

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

	virtual void beginWrite();
	virtual void endWrite();

	virtual void beginRead(void* _data, u32 _dataSize);
	virtual void endRead();

	virtual bool serialize(bool& _value, const char* _key = nullptr) = 0;
	virtual bool serialize(u8& _value, const char* _key = nullptr) = 0;
	virtual bool serialize(u32& _value, const char* _key = nullptr) = 0;
	virtual bool serialize(float& _value, const char* _key = nullptr) = 0;

	virtual bool beginSerializeArray(u32& _size, const char* _key = nullptr) = 0;
	virtual bool endSerializeArray() = 0;

	//virtual bool beginSerializeObject(const char* _key = nullptr) = 0;
	//virtual bool endSerializeObject() = 0;

	virtual void* getData() const = 0;
	virtual u32 getDataSize() const = 0;

	const char* getLastError() const;

//private:
	String m_lastError;
	SerializationMode m_mode = SerializationMode::NONE;
	Allocator* m_allocator = nullptr;
};


class YAELIB_API BinarySerializer : public Serializer
{
public:
	BinarySerializer(Allocator* _allocator = nullptr);
	virtual ~BinarySerializer();

	virtual void beginWrite() override;
	virtual void endWrite() override;

	virtual void beginRead(void* _data, u32 _dataSize) override;
	virtual void endRead() override;

	virtual bool serialize(bool& _value, const char* _id = nullptr) override;
	virtual bool serialize(u8& _value, const char* _id = nullptr) override;
	virtual bool serialize(u32& _value, const char* _id = nullptr) override;
	virtual bool serialize(float& _value, const char* _id = nullptr) override;

	void* getData() const override;
	u32 getDataSize() const override;

	// @NOTE: ce truc de size c'est pas très clair. Il faut passer la taille totale au début de l'écriture, et on peut pas vérifier que le même nombre d'éléments est écrit ou bien déduire carrément le nombre d'éléments à la fin de l'écriture...
	// Pour faire autrement il faudrait écrire du code relatif au tableau dans toutes les fonctions de serialization, ou bien avoir une API spéçifique pour serialiser des tableaux. Les deux solutions sont bof
	virtual bool beginSerializeArray(u32& _size, const char* _id = nullptr) override;
	virtual bool endSerializeArray() override;

	//virtual bool beginSerializeObject(const char* _key = nullptr) override;
	//virtual bool endSerializeObject() override;

//private:
	struct ArrayBinaryBuffer
	{
		u32 elementCount;
	};
	/*struct ObjectBinaryBuffer
	{
	};*/
	enum class BufferType : u8
	{
		PLAIN,
		ARRAY,
		OBJECT
	};
	struct Buffer
	{
		Buffer() {}

		BufferType type;
		u8* data = nullptr;
		u32 dataSize = 0;
		u32 cursor = 0;
		union
		{
			ArrayBinaryBuffer array;
			//ObjectBinaryBuffer object;
		};
	};

	bool _serializeData(void* _data, u32 _size, const char* _id = nullptr);
	void _growBuffer(Buffer& _buffer, u32 _addedSize);
	Buffer& _getTopBuffer();

	Array<Buffer> m_bufferStack;
};

} // namespace yae
