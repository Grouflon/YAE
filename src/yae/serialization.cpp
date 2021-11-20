#include "serialization.h"

#include <cstdlib>

#include <json/json.h>
#include <mirror/mirror.h>

#include <yae/program.h>
#include <yae/memory.h>

using namespace mirror;

namespace yae {

namespace jsonHelpers {

json_value_s* allocateValue(Allocator* _allocator)
{
	json_value_s* value = (json_value_s*)_allocator->allocate(sizeof(json_value_s));
	*value = {};
	value->type = json_type_null;
	return value;
}

void allocateNullPayload(Allocator* _allocator, json_value_s* _value)
{
	_value->type = json_type_null;
}

void allocateBoolPayload(Allocator* _allocator, json_value_s* _value, bool _payload)
{
	_value->type = _payload ? json_type_true : json_type_false;
}

void allocateNumberPayload(Allocator* _allocator, json_value_s* _value, i64 _payload)
{
	_value->type = json_type_number;
	json_number_s* number = (json_number_s*)_allocator->allocate(sizeof(json_number_s));
	char buffer[64];
	snprintf(buffer, countof(buffer) - 1, "%lld", _payload);
	size_t numberSize = strlen(buffer);
	number->number = (char*)_allocator->allocate(numberSize + 1);
	strcpy((char*)number->number, buffer);
	number->number_size = numberSize;
	_value->payload = number;
}

void allocateNumberPayload(Allocator* _allocator, json_value_s* _value, u64 _payload)
{
	_value->type = json_type_number;
	json_number_s* number = (json_number_s*)_allocator->allocate(sizeof(json_number_s));
	char buffer[64];
	snprintf(buffer, countof(buffer) - 1, "%llu", _payload);
	size_t numberSize = strlen(buffer);
	number->number = (char*)_allocator->allocate(numberSize + 1);
	strcpy((char*)number->number, buffer);
	number->number_size = numberSize;
	_value->payload = number;
}

void allocateNumberPayload(Allocator* _allocator, json_value_s* _value, double _payload)
{
	_value->type = json_type_number;
	json_number_s* number = (json_number_s*)_allocator->allocate(sizeof(json_number_s));
	char buffer[64];
	snprintf(buffer, countof(buffer) - 1, "%f", _payload);
	size_t numberSize = strlen(buffer);
	number->number = (char*)_allocator->allocate(numberSize + 1);
	strcpy((char*)number->number, buffer);
	number->number_size = numberSize;
	_value->payload = number;
}

json_string_s* allocateString(Allocator* _allocator, const char* _str)
{
	json_string_s* str = (json_string_s*)_allocator->allocate(sizeof(json_string_s));
	size_t strSize = strlen(_str);
	str->string = (char*)_allocator->allocate(strSize + 1);
	strcpy((char*)str->string, _str);
	str->string_size = strSize;
	return str;
}

void deallocateString(Allocator* _allocator, json_string_s* _str)
{
	_allocator->deallocate((void*)_str->string);
	_allocator->deallocate(_str);
}

void allocateStringPayload(Allocator* _allocator, json_value_s* _value, const char* _payload)
{
	_value->type = json_type_string;
	_value->payload = allocateString(_allocator, _payload);
}

json_object_s* allocateObjectPayload(Allocator* _allocator, json_value_s* _value)
{
	_value->type = json_type_object;
	json_object_s* object = (json_object_s*)_allocator->allocate(sizeof(json_object_s));
	*object = {};
	_value->payload = object;
	return object;
}

json_object_element_s* allocateObjectElement(Allocator* _allocator, json_object_s* _object, const char* _name, json_object_element_s* _lastElement)
{
	json_object_element_s* element = (json_object_element_s*)_allocator->allocate(sizeof(json_object_element_s));
	*element = {};
	if (_lastElement != nullptr)
	{
		YAE_ASSERT(_lastElement->next == nullptr);
		_lastElement->next = element;
	}
	else
	{
		YAE_ASSERT(_object->start == nullptr);
		_object->start = element;
	}
	element->name = allocateString(_allocator, _name);
	++_object->length;
	return element;
}

void deallocatePayload(Allocator* _allocator, json_value_s* _value); // forward declaration
void deallocateValue(Allocator* _allocator, json_value_s* _value)
{
	deallocatePayload(_allocator, _value);
	_allocator->deallocate(_value);
}

void deallocatePayload(Allocator* _allocator, json_value_s* _value)
{
	switch (_value->type)
	{
	case json_type_number:
	{
		json_number_s* number = (json_number_s*)_value->payload;
		_allocator->deallocate((void*)number->number);
	}
	break;
	case json_type_string:
	{
		json_string_s* str = (json_string_s*)_value->payload;
		_allocator->deallocate((void*)str->string);
	}
	break;

	case json_type_object:
	{
		json_object_s* object = (json_object_s*)_value->payload;
		json_object_element_s* element = object->start;
		while (element != nullptr)
		{
			json_object_element_s* nextElement = element->next;
			deallocateString(_allocator, element->name);
			deallocateValue(_allocator, element->value);
			_allocator->deallocate(element);
			element = nextElement;
		}
	}
	}
	_allocator->deallocate(_value->payload);
}

} // namespace jsonHelpers

JsonSerializer::JsonSerializer(Allocator* _allocator)
	: m_allocator(_allocator)
{
	if (m_allocator == nullptr)
	{
		m_allocator = &scratchAllocator();
	}
	YAE_ASSERT(m_allocator != nullptr);
}

JsonSerializer::~JsonSerializer()
{
	YAE_ASSERT_MSG(m_root == nullptr, "Serializer destroyed during a serialization process")
	free(m_writeData);
	m_writeData = nullptr;
	m_writeDataSize = 0;
}

void JsonSerializer::beginRead(const void* _buffer, size_t _bufferSize)
{
	YAE_ASSERT_MSG(m_root == nullptr, "Serializer is in the middle of another serialization process.");

	m_root = json_parse(_buffer, _bufferSize);
	YAE_ASSERT(m_root != nullptr);
	m_valueStack.push(m_root);
	m_isReading = true;
}

void JsonSerializer::endRead()
{
	m_valueStack.pop();
	YAE_ASSERT(m_valueStack.empty());
	free(m_root);
	m_root = nullptr;
	m_isReading = false;
}

void JsonSerializer::beginWrite()
{
	free(m_writeData);
	m_writeData = nullptr;
	m_writeDataSize = 0;

	YAE_ASSERT_MSG(m_root == nullptr, "Serializer is in the middle of another serialization process.");
	m_root = jsonHelpers::allocateValue(m_allocator);
	m_valueStack.push(m_root);
}

void JsonSerializer::endWrite(const void** _data, size_t* _dataSize)
{
	m_valueStack.pop();
	YAE_ASSERT(m_valueStack.empty());

	m_writeData = json_write_pretty(m_root, "\t", "\n", &m_writeDataSize);

	jsonHelpers::deallocateValue(m_allocator, m_root);
	m_root = nullptr;

	*_data = m_writeData;
	*_dataSize = m_writeDataSize;
}

void JsonSerializer::serialize(void* _data, const mirror::TypeDesc* _type)
{
	YAE_ASSERT_MSG(m_root != nullptr, "You must begin the serialization in read or write mode.");

	if (m_isReading)
	{
		_read(_data, _type);
	}
	else
	{
		_write(_data, _type);
	}
}

void JsonSerializer::_write(void* _data, const mirror::TypeDesc* _type)
{
	json_value_s* value = m_valueStack.top();
	switch (_type->getType())
	{
		case mirror::Type_Class:
		{
			const Class* clss = reinterpret_cast<const mirror::Class*>(_type);
			json_object_s* object = jsonHelpers::allocateObjectPayload(m_allocator, value);
			std::vector<ClassMember*> members;
			clss->getMembers(members, true);
			json_object_element_s* currentElement = object->start;
			for (ClassMember* member : members)
			{
				currentElement = jsonHelpers::allocateObjectElement(m_allocator, object, member->getName(), currentElement);
				currentElement->value = jsonHelpers::allocateValue(m_allocator);
				m_valueStack.push(currentElement->value);
				void* elementData = member->getInstanceMemberPointer(_data);
				_write(elementData, member->getType());
				m_valueStack.pop();
			}
		}
		break;
		case mirror::Type_bool:
		{	
			bool* ptr = (bool*)_data;
			jsonHelpers::allocateBoolPayload(m_allocator, value, *ptr);
		}
		break;
		case mirror::Type_char:
		{
			char* ptr = (char*)_data;
			char str[] = { *ptr, 0 };
			jsonHelpers::allocateStringPayload(m_allocator, value, str);
		}
		break;
		case mirror::Type_int8:
		{
			i8* ptr = (i8*)_data;
			i64 n = *ptr;
			jsonHelpers::allocateNumberPayload(m_allocator, value, n);
		}
		break;
		case mirror::Type_int16:
		{
			i16* ptr = (i16*)_data;
			i64 n = *ptr;
			jsonHelpers::allocateNumberPayload(m_allocator, value, n);
		}
		break;
		case mirror::Type_int32:
		{
			i32* ptr = (i32*)_data;
			i64 n = *ptr;
			jsonHelpers::allocateNumberPayload(m_allocator, value, n);
		}
		break;
		case mirror::Type_int64:
		{
			i64* ptr = (i64*)_data;
			jsonHelpers::allocateNumberPayload(m_allocator, value, *ptr);
		}
		break;
		case mirror::Type_uint8:
		{
			u8* ptr = (u8*)_data;
			u64 n = *ptr;
			jsonHelpers::allocateNumberPayload(m_allocator, value, n);
		}
		break;
		case mirror::Type_uint16:
		{
			u16* ptr = (u16*)_data;
			u64 n = *ptr;
			jsonHelpers::allocateNumberPayload(m_allocator, value, n);
		}
		break;
		case mirror::Type_uint32:
		{
			u32* ptr = (u32*)_data;
			u64 n = *ptr;
			jsonHelpers::allocateNumberPayload(m_allocator, value, n);
		}
		break;
		case mirror::Type_uint64:
		{
			u64* ptr = (u64*)_data;
			jsonHelpers::allocateNumberPayload(m_allocator, value, *ptr);
		}
		break;
		case mirror::Type_float:
		{
			float* ptr = (float*)_data;
			double n = *ptr;
			jsonHelpers::allocateNumberPayload(m_allocator, value, n);
		}
		break;
		case mirror::Type_double:
		{
			double* ptr = (double*)_data;
			jsonHelpers::allocateNumberPayload(m_allocator, value, *ptr);
		}
		break;
		case mirror::Type_Enum:
		case mirror::Type_std_string:
		case mirror::Type_std_vector:
		case mirror::Type_FixedSizeArray:
		default:
			YAE_ASSERT_MSG(false, "Not implemented");
		break;
	}
}

void JsonSerializer::_read(void* _data, const mirror::TypeDesc* _type)
{
	json_value_s* value = m_valueStack.top();
	switch (_type->getType())
	{
	case mirror::Type_Class:
	{
		const Class* clss = reinterpret_cast<const mirror::Class*>(_type);
		json_object_s* object = json_value_as_object(value);
		YAE_ASSERT(object != nullptr);

		json_object_element_s* element = object->start;
		while (element != nullptr)
		{
			const ClassMember* member = clss->findMemberByName(element->name->string, true);
			if (member)
			{
				void* memberData = member->getInstanceMemberPointer(_data);
				m_valueStack.push(element->value);
				_read(memberData, member->getType());
				m_valueStack.pop();
			}
			element = element->next;
		}
	}
	break;
	case mirror::Type_bool:
	{
		YAE_ASSERT(value->type == json_type_true || value->type == json_type_false);
		bool* ptr = (bool*)(_data);
		*ptr = json_value_is_true(value);
	}
	break;
	case mirror::Type_char:
	{
		json_string_s* str = json_value_as_string(value);
		YAE_ASSERT(str && str->string_size == 1);
		char* ptr = (char*)(_data);
		*ptr = *str->string;
	}
	break;
	case mirror::Type_int8:
	{
		json_number_s* number = json_value_as_number(value);
		YAE_ASSERT(number != nullptr);
		i8* ptr = (i8*)(_data);
		*ptr = i8(atoi(number->number));
	}
	break;
	case mirror::Type_int16:
	{
		json_number_s* number = json_value_as_number(value);
		YAE_ASSERT(number != nullptr);
		i16* ptr = (i16*)(_data);
		*ptr = i16(atoi(number->number));
	}
	break;
	case mirror::Type_int32:
	{
		json_number_s* number = json_value_as_number(value);
		YAE_ASSERT(number != nullptr);
		i32* ptr = (i32*)(_data);
		*ptr = i32(atol(number->number));
	}
	break;
	case mirror::Type_int64:
	{
		json_number_s* number = json_value_as_number(value);
		YAE_ASSERT(number != nullptr);
		i64* ptr = (i64*)(_data);
		*ptr = i64(atoll(number->number));
	}
	break;
	case mirror::Type_uint8:
	{
		json_number_s* number = json_value_as_number(value);
		YAE_ASSERT(number != nullptr);
		u8* ptr = (u8*)(_data);
		*ptr = u8(atoi(number->number));
	}
	break;
	case mirror::Type_uint16:
	{
		json_number_s* number = json_value_as_number(value);
		YAE_ASSERT(number != nullptr);
		u16* ptr = (u16*)(_data);
		*ptr = u16(atoi(number->number));
	}
	break;
	case mirror::Type_uint32:
	{
		json_number_s* number = json_value_as_number(value);
		YAE_ASSERT(number != nullptr);
		u32* ptr = (u32*)(_data);
		*ptr = u32(atol(number->number));
	}
	break;
	case mirror::Type_uint64:
	{
		json_number_s* number = json_value_as_number(value);
		YAE_ASSERT(number != nullptr);
		u64* ptr = (u64*)(_data);
		*ptr = u64(atoll(number->number));
	}
	break;
	case mirror::Type_float:
	{
		json_number_s* number = json_value_as_number(value);
		YAE_ASSERT(number != nullptr);
		float* ptr = (float*)(_data);
		*ptr = float(atof(number->number));
	}
	break;
	case mirror::Type_double:
	{
		json_number_s* number = json_value_as_number(value);
		YAE_ASSERT(number != nullptr);
		double* ptr = (double*)(_data);
		*ptr = double(atof(number->number));
	}
	break;
	case mirror::Type_Enum:
	case mirror::Type_std_string:
	case mirror::Type_std_vector:
	case mirror::Type_FixedSizeArray:
	default:
		YAE_ASSERT_MSG(false, "Not implemented");
		break;
	}
}

} // namespace yae
