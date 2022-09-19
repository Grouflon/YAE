#pragma once

#include <yae/types.h>
#include <yae/serialization/Serializer.h>
#include <yae/containers/Array.h>

struct json_value_s;

namespace yae {

class Allocator;

class YAELIB_API JsonSerializer : public Serializer
{
public:
	JsonSerializer(Allocator* _allocator = nullptr);
	virtual ~JsonSerializer();

	virtual void beginWrite() override;
	virtual void endWrite() override;
	void* getWriteData() const;
	u32 getWriteDataSize();

	bool parseSourceData(const void* _data, u32 _dataSize);
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

private:
	bool _selectNextValue(const char* _id, json_value_s** _outValue);

	DataArray<json_value_s*> m_valueStack;

	void* m_writeData = nullptr;
	u32 m_writeDataSize = 0;

	json_value_s* m_writeRootValue = nullptr;
	json_value_s* m_readRootValue = nullptr;
};

} // namespace yae
