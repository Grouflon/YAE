#pragma once

namespace yae {
template <typename T> class DataArray;
template <typename T> class Array;
template <typename T> struct ResourcePtr;
} // namespace yae

namespace mirror {

// ---- Array

class CORE_API ArrayType : public Type
{
	public:
		Type* getSubType() const { return mirror::GetType(m_subType); }

		// Type agnostic methods
		uint32_t getSize(void* _arrayPointer) const { return m_getSizeFunction(_arrayPointer); }
		void setSize(void* _arrayPointer, uint32_t _newSize) const { m_setSizeFunction(_arrayPointer, _newSize); }
		void* getData(void* _arrayPointer) const { return m_getDataFunction(_arrayPointer); }
		void* getDataAt(void* _arrayPointer, uint32_t _index) const { return (char*)getData(_arrayPointer) + _index * getSubType()->getSize(); }
		void erase(void* _arrayPointer, uint32_t _index, uint32_t _count) const { m_eraseFunction(_arrayPointer, _index, _count); }
		void swap(void* _arrayPointer, uint32_t _indexA, uint32_t _indexB) const { m_swapFunction(_arrayPointer, _indexA, _indexB); }

		// internal
		ArrayType(
			TypeID _subType,
			uint32_t(*_getSizeFunction)(void*),
			void(*_setSizeFunction)(void* , uint32_t),
			void*(*_getDataFunction)(void*),
			void(*_eraseFunction)(void*, uint32_t, uint32_t),
			void(*_swapFunction)(void*, uint32_t, uint32_t)
		)
			: Type(TypeInfo_Custom, "")
			, m_subType(_subType)
			, m_getSizeFunction(_getSizeFunction)
			, m_setSizeFunction(_setSizeFunction)
			, m_getDataFunction(_getDataFunction)
			, m_eraseFunction(_eraseFunction)
			, m_swapFunction(_swapFunction)
		{
		}

		virtual void init() override
		{
			// TypeID should not be resolved until the init phase as it is possible that the type does not exists yet.
			// That's why we are only solving the name here at init time
			char buf[512];
			snprintf(buf, sizeof(buf), "Array_%s", GetType(m_subType)->getName());
			setName(buf);
			setCustomTypeName("Array");
		}

		TypeID m_subType = UNDEFINED_TYPEID;
		uint32_t(*m_getSizeFunction)(void*);
		void(*m_setSizeFunction)(void* , uint32_t);
		void*(*m_getDataFunction)(void*);
		void(*m_eraseFunction)(void* , uint32_t, uint32_t);
		void(*m_swapFunction)(void* , uint32_t, uint32_t);
};

template <typename T>
struct CustomTypeFactory<yae::DataArray<T>>
{
	static Type* Create()
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

		auto erase = [](void* _arrayPointer, uint32_t _index, uint32_t _count)
		{
			yae::DataArray<T>* arrayPointer = (yae::DataArray<T>*)_arrayPointer;
			arrayPointer->erase(_index, _count);
		};

		auto swap = [](void* _arrayPointer, uint32_t _indexA, uint32_t _indexB)
		{
			yae::DataArray<T>* arrayPointer = (yae::DataArray<T>*)_arrayPointer;
			arrayPointer->swap(_indexA, _indexB);
		};

		GetType<T>();
		return new ArrayType(GetTypeID<T>(), getSize, setSize, getData, erase, swap);
	}
};

template <typename T>
struct CustomTypeFactory<yae::Array<T>>
{
	static Type* Create()
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

		auto erase = [](void* _arrayPointer, uint32_t _index, uint32_t _count)
		{
			yae::Array<T>* arrayPointer = (yae::Array<T>*)_arrayPointer;
			arrayPointer->erase(_index, _count);
		};

		auto swap = [](void* _arrayPointer, uint32_t _indexA, uint32_t _indexB)
		{
			yae::Array<T>* arrayPointer = (yae::Array<T>*)_arrayPointer;
			arrayPointer->swap(_indexA, _indexB);
		};

		GetType<T>();
		return new ArrayType(GetTypeID<T>(), getSize, setSize, getData, erase, swap);
	}
};


// ---- ResourcePtr

class CORE_API ResourcePtrType : public Type
{
	public:
		Type* getSubType() const { return mirror::FindTypeByID(m_subType); }

	// internal
		ResourcePtrType(TypeID _subType)
			: Type(TypeInfo_Custom, "")
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
struct CustomTypeFactory<yae::ResourcePtr<T>>
{
	static Type* Create()
	{
		return new ResourcePtrType(GetTypeID<T>());
	}
};

} // namespace mirror
