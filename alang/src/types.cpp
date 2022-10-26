#include "types.h"
#include <cstring>

namespace alang {

void *Value::GetValue() const 
{ 
	if (!_type)
		return nullptr;
	return _type->_size > sizeof(_value) ? (void *)_value : (void *)&_value; 
}

void Value::Delete()
{
	if (_type && _type->_size > sizeof(_value)) {
		free((void *)_value);
	}
	_type = nullptr;
	_value = 0;
}

void Value::Init(TypeDesc *type, void *val)
{
	if (!_type)
		return;
	void *thisVal;
	if (_type->_size > sizeof(_value)) {
		thisVal = malloc(_type->_size);
		_value = (uintptr_t)thisVal;
		ASSERT((_value % _type->_align) == 0);
	} else {
		thisVal = &_value;
	}
	if (val) {
		memcpy(thisVal, val, _type->_size);
	}
}

void Value::Copy(Value const &other)
{
	Delete();
	Init(other._type, other.GetValue());
}

void Value::Move(Value &&other)
{
	Delete();
	_type = other._type;
	_value = other._value;
	other._type = nullptr;
	other._value = 0;
}


TypeDesc::TypeDesc(String name, size_t size, size_t align)
	: Definition{ name }
	, _size{ size }
	, _align{ align }
{
}

}
