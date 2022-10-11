#include "JsonSerializer.h"

#include <yae/memory.h>

#include <json/json.h>

#include <cstdlib>
#include <cstring>

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

json_array_s* allocateArrayPayload(Allocator* _allocator, json_value_s* _value)
{
	_value->type = json_type_array;
	json_array_s* array = (json_array_s*)_allocator->allocate(sizeof(json_array_s));
	*array = {};
	_value->payload = array;
	return array;
}

json_array_element_s* allocateArrayElement(Allocator* _allocator, json_array_s* _array)
{
	json_array_element_s* element = (json_array_element_s*)_allocator->allocate(sizeof(json_array_element_s));
	*element = {};
	json_array_element_s* lastElement = _array->start;
	if (lastElement == nullptr)
	{
		_array->start = element;
	}
	else
	{
		while (lastElement->next != nullptr)
		{
			lastElement = lastElement->next;
		}
		lastElement->next = element;
	}
	++_array->length;
	return element;
}

json_object_s* allocateObjectPayload(Allocator* _allocator, json_value_s* _value)
{
	_value->type = json_type_object;
	json_object_s* object = (json_object_s*)_allocator->allocate(sizeof(json_object_s));
	*object = {};
	_value->payload = object;
	return object;
}

json_object_element_s* allocateObjectElement(Allocator* _allocator, json_object_s* _object, const char* _name)
{
	json_object_element_s* element = (json_object_element_s*)_allocator->allocate(sizeof(json_object_element_s));
	*element = {};
	json_object_element_s* lastElement = _object->start;
	if (lastElement == nullptr)
	{
		_object->start = element;
	}
	else
	{
		while (lastElement->next != nullptr)
		{
			lastElement = lastElement->next;
		}
		lastElement->next = element;
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
		json_object_s* object = json_value_as_object(_value);
		YAE_ASSERT(object != nullptr);
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
	break;

	case json_type_array:
	{
		json_array_s* array = json_value_as_array(_value);
		YAE_ASSERT(array != nullptr);
		json_array_element_s* element = array->start;
		while (element != nullptr)
		{
			json_array_element_s* nextElement = element->next;
			deallocateValue(_allocator, element->value);
			_allocator->deallocate(element);
			element = nextElement;
		}
	}
	break;
	}
	_allocator->deallocate(_value->payload);
}

void* jsonMalloc(void* _userData, size_t _size)
{
	JsonSerializer* serializer = (JsonSerializer*)_userData;
	return serializer->m_allocator->allocate(_size);
}

} // namespace jsonHelpers

JsonSerializer::JsonSerializer(Allocator* _allocator)
	: Serializer(_allocator)
{
}

JsonSerializer::~JsonSerializer()
{
	YAE_ASSERT_MSG(m_valueStack.size() == 0, "Serializer destroyed during a serialization process")
	m_allocator->deallocate(m_writeData);
}

void JsonSerializer::beginWrite()
{
	YAE_ASSERT_MSG(m_valueStack.size() == 0, "Serializer is in the middle of another serialization process.");
	YAE_ASSERT(m_writeRootValue == nullptr);

	Serializer::beginWrite();

	m_allocator->deallocate(m_writeData);
	m_writeData = nullptr;
	m_writeDataSize = 0;
}

void JsonSerializer::endWrite()
{
	YAE_ASSERT(m_valueStack.size() == 0);
	YAE_ASSERT(m_writeData == nullptr);

	json_value_s* root = m_writeRootValue;
	m_writeRootValue = nullptr;

	size_t writeDataSize;
	void* data = json_write_pretty(root, "\t", "\n", &writeDataSize);
	m_writeDataSize = (u32)writeDataSize - 1; // we dont want the trailing 0
	YAE_ASSERT(data != nullptr);
	m_writeData = m_allocator->allocate(m_writeDataSize);
	memcpy(m_writeData, data, m_writeDataSize);
	free(data); // We do not keep the data returned so the malloced memory do not stay around

	jsonHelpers::deallocateValue(m_allocator, root);

	Serializer::endWrite();
}

void* JsonSerializer::getWriteData() const
{
	YAE_ASSERT_MSG(getMode() == SerializationMode::NONE, "WriteData can't be queried during a serialization. This call should go outside of the begin/end block.");
	return m_writeData;
}

u32 JsonSerializer::getWriteDataSize()
{
	YAE_ASSERT_MSG(getMode() == SerializationMode::NONE, "WriteData can't be queried during a serialization. This call should go outside of the begin/end block.");
	return m_writeDataSize;
}

bool JsonSerializer::parseSourceData(const void* _data, u32 _dataSize)
{
	YAE_ASSERT_MSG(getMode() == SerializationMode::NONE, "Data can't be parsed during a serialization. This call should go outside of the begin/end block.");

	m_allocator->deallocate(m_readRootValue);

	json_parse_result_s result;
	m_readRootValue = json_parse_ex(
		_data, 
		_dataSize, 
		json_parse_flags_allow_json5,
		&jsonHelpers::jsonMalloc,
		this,
		&result
	);
	if (m_readRootValue == nullptr)
	{
		const char* errorString = "";
		switch(result.error)
		{
			case json_parse_error_none:
			errorString = "no error occurred (huzzah!).";
			break;

			case json_parse_error_expected_comma_or_closing_bracket:
			errorString = "expected either a comma or a closing '}' or ']' to close an object or array.";
			break;

			case json_parse_error_expected_colon:
			errorString = "colon separating name/value pair was missing.";
			break;

			case json_parse_error_expected_opening_quote:
			errorString = "expected string to begin with '\"'.";
			break;

			case json_parse_error_invalid_string_escape_sequence:
			errorString = "invalid escaped sequence in string.";
			break;

			case json_parse_error_invalid_number_format:
			errorString = "invalid number format.";
			break;

			case json_parse_error_invalid_value:
			errorString = "invalid value.";
			break;

			case json_parse_error_premature_end_of_buffer:
			errorString = "reached end of buffer before object/array was complete.";
			break;

			case json_parse_error_invalid_string:
			errorString = "string was malformed.";
			break;

			case json_parse_error_allocator_failed:
			errorString = "a call to malloc, or a user provider allocator, failed.";
			break;

			case json_parse_error_unexpected_trailing_characters:
			errorString = "the JSON input had unexpected trailing characters that weren't part of the JSON value.";
			break;

			/* catch-all error for everything else that exploded (real bad chi!). */
			case json_parse_error_unknown:
			errorString = "unknown error.";
			break;
		}

		m_lastError = string::format("JSON parsing error, line %d, row %d: %s", result.error_line_no, result.error_row_no, errorString);

		return false;
	}
	return true;
}

void JsonSerializer::beginRead()
{
	YAE_ASSERT_MSG(m_valueStack.size() == 0, "Serializer is in the middle of another serialization process.");
	YAE_ASSERT_MSG(m_readRootValue != nullptr, "No parsed data is available. Have you called parseSourceData?");

	Serializer::beginRead();
	
}

void JsonSerializer::endRead()
{
	YAE_ASSERT(m_valueStack.size() == 0);

	Serializer::endRead();

	m_allocator->deallocate(m_readRootValue);
	m_readRootValue = nullptr;
}

bool JsonSerializer::serialize(bool& _value, const char* _id)
{
	json_value_s* value;
	if (!_selectNextValue(_id, &value))
	{
		return false;
	}
	YAE_ASSERT(value);

	if (getMode() == SerializationMode::WRITE)
	{
		jsonHelpers::allocateBoolPayload(m_allocator, value, _value);
	}
	else if (getMode() == SerializationMode::READ)
	{
		if (value->type != json_type_true && value->type != json_type_false)
		{
			m_lastError = "Current value is not a bool type";
			return false;
		}
		_value = json_value_is_true(value);
	}
	return true;
}

bool JsonSerializer::serialize(u8& _value, const char* _id)
{
	json_value_s* value;
	if (!_selectNextValue(_id, &value))
	{
		return false;
	}
	YAE_ASSERT(value);

	if (getMode() == SerializationMode::WRITE)
	{
		u64 number = _value;
		jsonHelpers::allocateNumberPayload(m_allocator, value, number);
	}
	else if (getMode() == SerializationMode::READ)
	{
		json_number_s* number = json_value_as_number(value);
		if (number == nullptr)
		{
			m_lastError = "Current value is not a number type";
			return false;
		}
		_value = u8(atoi(number->number));
	}
	return true;
}

bool JsonSerializer::serialize(u16& _value, const char* _id)
{
	json_value_s* value;
	if (!_selectNextValue(_id, &value))
	{
		return false;
	}
	YAE_ASSERT(value);

	if (getMode() == SerializationMode::WRITE)
	{
		u64 number = _value;
		jsonHelpers::allocateNumberPayload(m_allocator, value, number);
	}
	else if (getMode() == SerializationMode::READ)
	{
		json_number_s* number = json_value_as_number(value);
		if (number == nullptr)
		{
			m_lastError = "Current value is not a number type";
			return false;
		}
		_value = u16(atoi(number->number));
	}
	return true;
}

bool JsonSerializer::serialize(u32& _value, const char* _id)
{
	json_value_s* value;
	if (!_selectNextValue(_id, &value))
	{
		return false;
	}
	YAE_ASSERT(value);

	if (getMode() == SerializationMode::WRITE)
	{
		u64 number = _value;
		jsonHelpers::allocateNumberPayload(m_allocator, value, number);
	}
	else if (getMode() == SerializationMode::READ)
	{
		json_number_s* number = json_value_as_number(value);
		if (number == nullptr)
		{
			m_lastError = "Current value is not a number type";
			return false;
		}
		_value = u32(atoi(number->number));
	}
	return true;
}

bool JsonSerializer::serialize(u64& _value, const char* _id)
{
	json_value_s* value;
	if (!_selectNextValue(_id, &value))
	{
		return false;
	}
	YAE_ASSERT(value);

	if (getMode() == SerializationMode::WRITE)
	{
		u64 number = _value;
		jsonHelpers::allocateNumberPayload(m_allocator, value, number);
	}
	else if (getMode() == SerializationMode::READ)
	{
		json_number_s* number = json_value_as_number(value);
		if (number == nullptr)
		{
			m_lastError = "Current value is not a number type";
			return false;
		}
		_value = u64(atoi(number->number));
	}
	return true;
}

bool JsonSerializer::serialize(i8& _value, const char* _id)
{
	json_value_s* value;
	if (!_selectNextValue(_id, &value))
	{
		return false;
	}
	YAE_ASSERT(value);

	if (getMode() == SerializationMode::WRITE)
	{
		i64 number = _value;
		jsonHelpers::allocateNumberPayload(m_allocator, value, number);
	}
	else if (getMode() == SerializationMode::READ)
	{
		json_number_s* number = json_value_as_number(value);
		if (number == nullptr)
		{
			m_lastError = "Current value is not a number type";
			return false;
		}
		_value = i8(atoi(number->number));
	}
	return true;
}

bool JsonSerializer::serialize(i16& _value, const char* _id)
{
	json_value_s* value;
	if (!_selectNextValue(_id, &value))
	{
		return false;
	}
	YAE_ASSERT(value);

	if (getMode() == SerializationMode::WRITE)
	{
		i64 number = _value;
		jsonHelpers::allocateNumberPayload(m_allocator, value, number);
	}
	else if (getMode() == SerializationMode::READ)
	{
		json_number_s* number = json_value_as_number(value);
		if (number == nullptr)
		{
			m_lastError = "Current value is not a number type";
			return false;
		}
		_value = i16(atoi(number->number));
	}
	return true;
}

bool JsonSerializer::serialize(i32& _value, const char* _id)
{
	json_value_s* value;
	if (!_selectNextValue(_id, &value))
	{
		return false;
	}
	YAE_ASSERT(value);

	if (getMode() == SerializationMode::WRITE)
	{
		i64 number = _value;
		jsonHelpers::allocateNumberPayload(m_allocator, value, number);
	}
	else if (getMode() == SerializationMode::READ)
	{
		json_number_s* number = json_value_as_number(value);
		if (number == nullptr)
		{
			m_lastError = "Current value is not a number type";
			return false;
		}
		_value = i32(atoi(number->number));
	}
	return true;
}

bool JsonSerializer::serialize(i64& _value, const char* _id)
{
	json_value_s* value;
	if (!_selectNextValue(_id, &value))
	{
		return false;
	}
	YAE_ASSERT(value);

	if (getMode() == SerializationMode::WRITE)
	{
		i64 number = _value;
		jsonHelpers::allocateNumberPayload(m_allocator, value, number);
	}
	else if (getMode() == SerializationMode::READ)
	{
		json_number_s* number = json_value_as_number(value);
		if (number == nullptr)
		{
			m_lastError = "Current value is not a number type";
			return false;
		}
		_value = i64(atoi(number->number));
	}
	return true;
}

bool JsonSerializer::serialize(float& _value, const char* _id)
{
	json_value_s* value;
	if (!_selectNextValue(_id, &value))
	{
		return false;
	}
	YAE_ASSERT(value);

	if (getMode() == SerializationMode::WRITE)
	{
		float number = _value;
		jsonHelpers::allocateNumberPayload(m_allocator, value, number);
	}
	else if (getMode() == SerializationMode::READ)
	{
		json_number_s* number = json_value_as_number(value);
		if (number == nullptr)
		{
			m_lastError = "Current value is not a number type";
			return false;
		}
		_value = float(atof(number->number));
	}
	return true;
}

bool JsonSerializer::serialize(double& _value, const char* _id)
{
	json_value_s* value;
	if (!_selectNextValue(_id, &value))
	{
		return false;
	}
	YAE_ASSERT(value);

	if (getMode() == SerializationMode::WRITE)
	{
		double number = _value;
		jsonHelpers::allocateNumberPayload(m_allocator, value, number);
	}
	else if (getMode() == SerializationMode::READ)
	{
		json_number_s* number = json_value_as_number(value);
		if (number == nullptr)
		{
			m_lastError = "Current value is not a number type";
			return false;
		}
		_value = double(atoi(number->number));
	}
	return true;
}

bool JsonSerializer::beginSerializeArray(u32& _size, const char* _id)
{
	json_value_s* value;
	if (!_selectNextValue(_id, &value))
	{
		return false;
	}
	YAE_ASSERT(value);

	if (getMode() == SerializationMode::WRITE)
	{
		jsonHelpers::allocateArrayPayload(m_allocator, value);
		m_valueStack.push_back(value);
	}
	else if (getMode() == SerializationMode::READ)
	{
		if (value->type != json_type_array)
		{
			m_lastError = "Current value is not an array type";
			return false;	
		}
		json_array_s* array = json_value_as_array(value);
		_size = array->length;
		array->length = 0; // We use the length field to keep track of what element is to read next
		m_valueStack.push_back(value);
	}
	return true;
}

bool JsonSerializer::endSerializeArray()
{
	YAE_ASSERT(getMode() != SerializationMode::NONE);
	json_value_s* value = m_valueStack.back();
	YAE_ASSERT(value->type == json_type_array);

	if (getMode() == SerializationMode::READ)
	{
		// Put the correct length back in the object
		size_t length = 0;
		json_array_s* array = json_value_as_array(value);
		json_array_element_s* currentElement = array->start;
		while (currentElement != nullptr)
		{
			++length;
			currentElement = currentElement->next;
		}
		array->length = length;
	}

	m_valueStack.pop_back();
	return true;
}

bool JsonSerializer::beginSerializeObject(const char* _id)
{
	json_value_s* value;
	if (!_selectNextValue(_id, &value))
	{
		return false;
	}
	YAE_ASSERT(value);

	if (getMode() == SerializationMode::WRITE)
	{
		jsonHelpers::allocateObjectPayload(m_allocator, value);
		m_valueStack.push_back(value);
	}
	else if (getMode() == SerializationMode::READ)
	{
		if (value->type != json_type_object)
		{
			m_lastError = "Current value is not an object type";
			return false;	
		}
		m_valueStack.push_back(value);
	}
	return true;
}

bool JsonSerializer::endSerializeObject()
{
	YAE_ASSERT(getMode() != SerializationMode::NONE);
	json_value_s* value = m_valueStack.back();
	YAE_ASSERT(value->type == json_type_object);
	m_valueStack.pop_back();
	return true;
}


bool JsonSerializer::_selectNextValue(const char* _id, json_value_s** _outValue)
{
	YAE_ASSERT(getMode() != SerializationMode::NONE);
	YAE_ASSERT(_outValue != nullptr);

	if (_id != nullptr)
	{
		if (m_valueStack.size() == 0 || m_valueStack.back()->type != json_type_object)
		{
			m_lastError = "Cannot serialize item with id outside of any object scope.";
			return false;	
		}
	}

	// write
	if (getMode() == SerializationMode::WRITE)
	{
		if (m_writeRootValue == nullptr)
		{
			m_writeRootValue = jsonHelpers::allocateValue(m_allocator);
			*_outValue = m_writeRootValue;
			return true;
		}
		else if (m_valueStack.size() == 0)
		{
			m_lastError = "Cant write the root value twice";
			return false;
		}

		json_value_s* topValue = m_valueStack.back();
		json_value_s* toWriteValue = nullptr;
		switch(topValue->type)
		{
			case json_type_array:
			{
				json_array_s* array = (json_array_s*)topValue->payload;
				json_array_element_s* currentElement = jsonHelpers::allocateArrayElement(m_allocator, array);
				toWriteValue = jsonHelpers::allocateValue(m_allocator);
				currentElement->value = toWriteValue;
			}
			break;

			case json_type_object:
			{
				YAE_ASSERT(_id != nullptr);

				json_object_s* object = (json_object_s*)topValue->payload;
				json_object_element_s* currentElement = jsonHelpers::allocateObjectElement(m_allocator, object, _id);
				toWriteValue = jsonHelpers::allocateValue(m_allocator);
				currentElement->value = toWriteValue;
			}
			break;

			default:
			{
				m_lastError = "Cannot serialize loose item outside of an array, an object, or the root of the json document.";
				return false;
			}
			break;
		}

		YAE_ASSERT(toWriteValue != nullptr);
		*_outValue = toWriteValue;
	}
	// read
	else if (getMode() == SerializationMode::READ)
	{
		if (m_valueStack.size() == 0)
		{
			*_outValue = m_readRootValue;
			return true;
		}

		json_value_s* topValue = m_valueStack.back();
		json_value_s* toReadValue = nullptr;
		switch(topValue->type)
		{
			case json_type_array:
			{
				json_array_s* array = (json_array_s*)topValue->payload;
				json_array_element_s* currentElement = array->start;
				YAE_ASSERT(currentElement != nullptr);
				for (size_t i = 0; i < array->length; ++i)
				{
					currentElement = currentElement->next;
					YAE_ASSERT(currentElement != nullptr);
				}
				toReadValue = currentElement->value;
				++array->length; // length is used to keep track of current element
			}
			break;

			case json_type_object:
			{
				YAE_ASSERT(_id != nullptr);

				json_object_s* object = (json_object_s*)topValue->payload;
				json_object_element_s* currentElement = object->start;
				while (currentElement != nullptr && strcmp(currentElement->name->string, _id) != 0)
				{
					currentElement = currentElement->next;
				}
				if (currentElement == nullptr)
				{
					m_lastError = string::format("Unable to find field with id \"%s\"", _id);
					return false;
				}
				toReadValue = currentElement->value;
			}
			break;

			default:
			{
				toReadValue = topValue;
			}
			break;
		}
		YAE_ASSERT(toReadValue != nullptr);
		*_outValue = toReadValue;
	}
	return true;
}

/*
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
	case mirror::Type_FixedSizeArray:
	default:
		YAE_ASSERT_MSG(false, "Not implemented");
		break;
	}
}
*/

} // namespace yae
