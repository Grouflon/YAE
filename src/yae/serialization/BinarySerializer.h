#pragma once

#include <yae/types.h>

#include <yae/hash.h>
#include <yae/containers/Array.h>
#include <yae/containers/HashMap.h>

#include <yae/serialization/Serializer.h>

namespace yae {

class YAE_API BinarySerializer : public Serializer
{
public:
	BinarySerializer(Allocator* _allocator = nullptr);
	virtual ~BinarySerializer();

	virtual void beginWrite() override;
	virtual void endWrite() override;
	void* getWriteData() const;
	u32 getWriteDataSize() const;

	void setReadData(void* _data, u32 _dataSize);
	virtual void beginRead() override;
	virtual void endRead() override;

	virtual bool serialize(bool& _value, const char* _id = nullptr) override;
	virtual bool serialize(u8& _value, const char* _id = nullptr) override;
	virtual bool serialize(u16& _value, const char* _id = nullptr) override;
	virtual bool serialize(u32& _value, const char* _id = nullptr) override;
	virtual bool serialize(u64& _value, const char* _id = nullptr) override;
	virtual bool serialize(i8& _value, const char* _id = nullptr) override;
	virtual bool serialize(i16& _value, const char* _id = nullptr) override;
	virtual bool serialize(i32& _value, const char* _id = nullptr) override;
	virtual bool serialize(i64& _value, const char* _id = nullptr) override;
	virtual bool serialize(float& _value, const char* _id = nullptr) override;
	virtual bool serialize(double& _value, const char* _id = nullptr) override;

	// @NOTE: ce truc de size c'est pas très clair. Il faut passer la taille totale au début de l'écriture, et on peut pas vérifier que le même nombre d'éléments est écrit ou bien déduire carrément le nombre d'éléments à la fin de l'écriture...
	// Pour faire autrement il faudrait écrire du code relatif au tableau dans toutes les fonctions de serialization, ou bien avoir une API spéçifique pour serialiser des tableaux. Les deux solutions sont bof
	virtual bool beginSerializeArray(u32& _size, const char* _id = nullptr) override;
	virtual bool endSerializeArray() override;

	virtual bool beginSerializeObject(const char* _id = nullptr) override;
	virtual bool endSerializeObject() override;

//private:
	struct ArrayBinaryBuffer
	{
		ArrayBinaryBuffer() {}
		u32 elementCount;
	};
	struct ObjectBinaryBuffer
	{
		ObjectBinaryBuffer() {}
		ObjectBinaryBuffer(Allocator* _allocator) : index(_allocator) {}
		HashMap<StringHash, u32> index;
	};
	enum class BufferType : u8
	{
		PLAIN,
		ARRAY,
		OBJECT
	};
	struct Buffer
	{
		Buffer() {}
		Buffer(Allocator* _allocator) : object(_allocator) {}

		BufferType type;
		u8* data = nullptr;
		u32 dataSize = 0;
		u32 cursor = 0;
		u32 maxCursor = 0;
		ArrayBinaryBuffer array;
		ObjectBinaryBuffer object;
	};

	bool _serializeData(void* _data, u32 _size, const char* _id = nullptr);
	bool _serializeDataRaw(void* _data, u32 _size, const char* _id = nullptr); // do not write size in the buffer
	bool _processId(const char* _id);
	void _growBuffer(Buffer& _buffer, u32 _addedSize);
	void _setBufferCursor(Buffer& _buffer, u32 _cursor);
	Buffer& _getTopBuffer();

	Array<Buffer> m_bufferStack;
	void* m_writeData = nullptr;
	u32 m_writeDataSize = 0;
	void* m_readData = nullptr;
	u32 m_readDataSize = 0;
};

} // namespace yae
