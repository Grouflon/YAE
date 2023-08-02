#pragma once

namespace yae {
template <typename T> class DataArray;
template <typename T> class Array;
template <typename T> class ResourcePtr;
} // namespace yae

namespace mirror {

// ---- Array

class YAE_API ArrayType : public TypeDesc
{
	public:
		TypeDesc* getSubType() const { return mirror::FindTypeByID(m_subType); }

		uint32_t getSize(void* _arrayPointer) const { return m_getSizeFunction(_arrayPointer); }
		void setSize(void* _arrayPointer, uint32_t _newSize) const { m_setSizeFunction(_arrayPointer, _newSize); }
		void* getData(void* _arrayPointer) const { return m_getDataFunction(_arrayPointer); }

	// internal
		ArrayType(
			TypeID _subType,
			uint32_t(*_getSizeFunction)(void*),
			void(*_setSizeFunction)(void* , uint32_t),
			void*(*_getDataFunction)(void*)
		)
			: TypeDesc(Type_Custom, "")
			, m_subType(_subType)
			, m_getSizeFunction(_getSizeFunction)
			, m_setSizeFunction(_setSizeFunction)
			, m_getDataFunction(_getDataFunction)
		{
			char buf[512];
			snprintf(buf, sizeof(buf), "Array_%s", GetTypeSet().findTypeByID(_subType)->getName());
			setName(buf);
			setCustomTypeName("Array");
		}

		TypeID m_subType = UNDEFINED_TYPEID;
		uint32_t(*m_getSizeFunction)(void*);
		void(*m_setSizeFunction)(void* , uint32_t);
		void*(*m_getDataFunction)(void*);
};

template <typename T>
struct CustomTypeDescFactory<yae::DataArray<T>>
{
	static TypeDesc* Create()
	{
		auto getSize = [](void* _arrayPointer) -> uint32_t
		{
			yae::DataArray<T>* arrayPointer = (yae::DataArray<T>*)_arrayPointer;
			return arrayPointer->size();
		};

		auto setSize = [](void* _arrayPointer, uint32_t _newSize)
		{
			yae::DataArray<T>* arrayPointer = (yae::DataArray<T>*)_arrayPointer;
			arrayPointer->resize(_newSize);
		};

		auto getData = [](void* _arrayPointer) -> void*
		{
			yae::DataArray<T>* arrayPointer = (yae::DataArray<T>*)_arrayPointer;
			return arrayPointer->data();
		};

		return new ArrayType(GetTypeID<T>(), getSize, setSize, getData);
	}
};

template <typename T>
struct CustomTypeDescFactory<yae::Array<T>>
{
	static TypeDesc* Create()
	{
		auto getSize = [](void* _arrayPointer) -> uint32_t
		{
			yae::Array<T>* arrayPointer = (yae::Array<T>*)_arrayPointer;
			return arrayPointer->size();
		};

		auto setSize = [](void* _arrayPointer, uint32_t _newSize)
		{
			yae::Array<T>* arrayPointer = (yae::Array<T>*)_arrayPointer;
			arrayPointer->resize(_newSize);
		};

		auto getData = [](void* _arrayPointer) -> void*
		{
			yae::Array<T>* arrayPointer = (yae::Array<T>*)_arrayPointer;
			return arrayPointer->data();
		};

		return new ArrayType(GetTypeID<T>(), getSize, setSize, getData);
	}
};


// ---- ResourcePtr

class YAE_API ResourcePtrType : public TypeDesc
{
	public:
		TypeDesc* getSubType() const { return mirror::FindTypeByID(m_subType); }

	// internal
		ResourcePtrType(TypeID _subType)
			: TypeDesc(Type_Custom, "")
			, m_subType(_subType)
		{
			char buf[512];
			snprintf(buf, sizeof(buf), "ResourcePtr_%s", GetTypeSet().findTypeByID(_subType)->getName());
			setName(buf);
			setCustomTypeName("ResourcePtr");
		}

		TypeID m_subType = UNDEFINED_TYPEID;
};

template <typename T>
struct CustomTypeDescFactory<yae::ResourcePtr<T>>
{
	static TypeDesc* Create()
	{
		return new ResourcePtrType(GetTypeID<T>());
	}
};

} // namespace mirror
