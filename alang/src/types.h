#pragma once

#include "definitions.h"

namespace alang {

struct TypeDesc;
struct Value {
	TypeDesc *_type = nullptr;
	uintptr_t _value = 0;  // holds the value if _type->_size <= sizeof(uintptr_t), or a pointer to it otherwise

	Value() = default;
	Value(TypeDesc *type, void *val) { Init(type, val); }
	Value(Value const &other) { Copy(other); }
	Value(Value &&other) { Move(std::move(other)); }
	~Value() { Delete(); }

	Value &operator=(Value const &other) { Copy(other); return *this; }
	Value &operator=(Value &&other) { Move(std::move(other)); return *this; }

	void *GetValue() const;

	String ToString() const;

private:
	void Delete();
	void Init(TypeDesc *type, void *val);
	void Copy(Value const &other);
	void Move(Value &&other);
};

struct Module;
struct TypeDesc : public Definition {
	struct Member {
		String _name;
		TypeDesc *_type = nullptr;
		size_t _offset = 0;
	};

	size_t _size = 0, _align = 0;
	TypeDesc *_genericDefinition = nullptr;
	std::vector<Value> _params; // generic params
	std::vector<Member> _members;

	TypeDesc(String name, size_t size, size_t align);
	TypeDesc() = default;

	Error Init(ParseNode const *node) override;

	rtti::TypeInfo const *GetTypeInfo() const override;

	static String GetGenericName(TypeDesc *genericDef, std::vector<Value> const &params);
};

}

namespace rtti {
template <> inline TypeInfo const *Get<alang::TypeDesc>() { return GetBases<alang::TypeDesc, alang::Definition>(); }
}