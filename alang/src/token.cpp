#include "token.h"

#include <algorithm>
#include <charconv>
#include "dbg.h"
#include "error.h"

namespace alang {

static std::vector<std::pair<Token::Type, char const *>> s_tokenTypeNames = {
	{ Token::Type::Invalid,     "Invalid"    },
	{ Token::Type::Keyword,	    "Keyword"	 },
	{ Token::Type::Separator,   "Separator"  },
	{ Token::Type::Identifier,  "Identifier" },
	{ Token::Type::String,	    "String"	 },
	{ Token::Type::Char,	    "Char"	     },
	{ Token::Type::Integer,	    "Integer"    },
	{ Token::Type::Double,	    "Double"	 },
};

static char const *GetTokenTypeName(Token::Type type)
{
	auto it = std::find_if(s_tokenTypeNames.begin(), s_tokenTypeNames.end(), [&](auto tn) { return tn.first == type; });
	return it == s_tokenTypeNames.end() ? "UNKNOWN" : it->second;
}

auto Token::GetClass() const -> Class
{
	switch (_type) {
		case Type::Invalid:
			return Class::Invalid;
		case Type::Keyword:
		case Type::Separator:
			return Class::Key;
		case Type::Identifier:
			return Class::Identifier;
		case Type::Char:
		case Type::String:
		case Type::Integer:
		case Type::Double:
			return Class::Literal;
		default:
			ASSERT(0);
			return Class::Invalid;
	}
}

std::string Token::Dump() const
{
	std::string result;
	result += std::string("Loc: ") + std::to_string(_filePos._line) + ":" + std::to_string(_filePos._posOnLine);
	result += std::string(" Type: ") + GetTokenTypeName(_type);
	result += std::string(" Str: \"") + _str + "\"";
	if (_type == Token::Type::Integer || _type == Token::Type::Char) {
		result += std::string(" Int: ") + std::to_string(_integer);
	} else if (_type == Token::Type::Double) {
		result += std::string(" Double: ") + std::to_string(_double);
	}
	return result;
}


static std::vector<uint32_t> GetUniqueCPs(std::vector<String> const &strs)
{
	std::vector<uint32_t> result;
	for (auto &s : strs) {
		uint8_t *p = (uint8_t *)&s.front();
		uint8_t *end = (uint8_t *)&s.back();
		while (p <= end) {
			uint32_t cp = utf8::read_cp(p);
			ASSERT(cp != utf8::InvalidCP);
			result.push_back(cp);
		}
	}
	std::sort(result.begin(), result.end());
	result.resize(std::unique(result.begin(), result.end()) - result.begin());
	return result;
}


Tokenizer::Tokenizer(std::string filePath, std::vector<String> const &keywordsSeparators)
	: _error(String(), {filePath, 0, 0})
{
	InitKeywordsSeparators(keywordsSeparators);

	_buffer.resize(32768);
	_file.rdbuf()->pubsetbuf((char*)_buffer.data(), _buffer.size());
	_file.open(filePath, std::ios::binary | std::ios::ate | std::ios::in);
	_fileSize = _file.tellg();
	_file.seekg(0, std::ios::beg);

	NextCP();
	if (_curCP == utf8::BomCP) {
		NextCP();
	}
	_error._location._posOnLine = 0;
}

Token const &Tokenizer::Current() const
{
	return _curToken;
}

bool Tokenizer::MoveNext()
{
	ParseToken(_curToken);
	return _curToken._type != Token::Type::Invalid;
}

void Tokenizer::InitKeywordsSeparators(std::vector<String> const &keywordsSeparators)
{
	for (auto &key : keywordsSeparators) {
		if (IsLetter(key[0])) {
			_keywords.push_back(key);
		} else {
			_separators.push_back(key);
		}
	}

	std::sort(_keywords.begin(), _keywords.end());
	std::sort(_separators.begin(), _separators.end());
	_separatorCPs = GetUniqueCPs(_separators);
}

bool Tokenizer::ParseToken(Token &token)
{
	PosInFile startPos;
	while (true) {
		startPos = _error._location;
		if (_curCP == utf8::InvalidCP) {
			if (_file.eof()) {
				token._type = Token::Type::Invalid;
				token._str.clear();
			} else {
				token._type = Token::Type::Invalid;
				token._str = _error._error;
			}
		} else if (_curCP == '#') {
			if (SkipComment())
				continue;
			token._type = Token::Type::Invalid;
			token._str = Err::NonClosedComment;
		} else if (IsWhitespace(_curCP)) {
			if (SkipWhitespace())
				continue;
			ASSERT(!"Skipping whitespace shouldn't fail");
		} else if (IsLetter(_curCP)) {
			bool res = ParseIdentifier(token);
			ASSERT(res);
		} else if (IsNumber(_curCP)) {
			bool res = ParseNumber(token);
			ASSERT(res);
		} else if (IsSeparator(_curCP)) {
			if (!ParseSeparator(token)) {
				token._type = Token::Type::Invalid;
				token._str = Err::UnrecognizedSeparator;
			}
		} else if (_curCP == '\'') {
			if (!ParseChar(token)) {
				token._type = Token::Type::Invalid;
				token._str = Err::InvalidCharLiteral;
			}
		} else if (_curCP == '"') {
			if (!ParseString(token)) {
				token._type = Token::Type::Invalid;
				token._str = Err::InvalidStringLiteral;
			}
		}

		break;
	}
	if (token._type == Token::Type::Invalid)
		_error._error = token._str;
	token._filePos = startPos;
	return token._type != Token::Type::Invalid;
}

bool Tokenizer::ParseIdentifier(Token &token)
{
	if (!IsLetter(_curCP))
		return false;
	token._str.clear();
	do {
		AppendCP(token._str, _curCP);
		NextCP();
	} while (IsLetter(_curCP) || IsNumber(_curCP));

	auto it = std::lower_bound(_keywords.begin(), _keywords.end(), token._str);
	if (it != _keywords.end() && *it == token._str) {
		token._type = Token::Type::Keyword;
	} else {
		token._type = Token::Type::Identifier;
	}

	return true;
}

bool Tokenizer::ParseSeparator(Token &token)
{
	token._str.clear();
	while (IsSeparator(_curCP)) {
		uint32_t len = (uint32_t)token._str.length();
		AppendCP(token._str, _curCP);
		auto it = std::lower_bound(_separators.begin(), _separators.end(), token._str);
		if (it == _separators.end() || it->rfind(token._str, 0) != 0) {
			token._str.resize(len);
			break;
		}
		NextCP();
	}
	if (token._str.empty())
		return false;
	token._type = Token::Type::Separator;
	return true;
}

bool Tokenizer::ParseString(Token &token)
{
	// TODO: escape sequences
	if (_curCP != '"')
		return false;
	token._str.clear();
	NextCP(); // skip "
	while (_curCP != '"') {
		if (_curCP == utf8::InvalidCP)
			return false;
		AppendCP(token._str, _curCP);
		NextCP();
	}
	NextCP(); // skip "
	token._type = Token::Type::String;
	return true;
}

bool Tokenizer::ParseChar(Token &token)
{
	// TODO: escape sequences
	if (_curCP != '\'')
		return false;
	NextCP();
	if (_curCP == '\'' || _curCP == utf8::InvalidCP)
		return false;
	token._str.clear();
	AppendCP(token._str, _curCP);
	token._integer = _curCP;
	NextCP();
	if (_curCP != '\'')
		return false;
	NextCP();
	token._type = Token::Type::Char;
	return true;
}

bool Tokenizer::ParseNumber(Token &token)
{
	// TODO: base N literals, exponents, proper error checking
	token._str.clear();
	while (IsNumber(_curCP)) {
		AppendCP(token._str, _curCP);
		NextCP();
	}
	if (token._str.empty())
		return false;
	if (_curCP == '.') {
		uint32_t wholeLen = (uint32_t)token._str.length();
		do {
			AppendCP(token._str, _curCP);
			NextCP();
		} while (IsNumber(_curCP));
		if (wholeLen + 1 == token._str.length()) {
			// nothing after the decimal dot
			return false;
		}
		std::from_chars(&token._str.front(), &token._str.back() + 1, token._double);
		token._type = Token::Type::Double;
		return true;
	}
	std::from_chars(&token._str.front(), &token._str.back() + 1, token._integer, 10);
	token._type = Token::Type::Integer;
	return true;
}

bool Tokenizer::SkipWhitespace()
{
	while (IsWhitespace(_curCP)) {
		NextCP();
	}
	return true;
}

bool Tokenizer::SkipComment()
{
	// just line comments for now
	while (_curCP != '\n' && _curCP != utf8::InvalidCP) {
		NextCP();
	}
	return true;
}

uint32_t Tokenizer::NextCP()
{
	uint8_t buf[4];
	if (!_file.get((char &)buf[0])) {
		return _curCP = utf8::InvalidCP;
	}
	uint8_t len = utf8::cp_size(buf);
	if (len > 1) {
		if (!_file.get((char *)buf + 1, len - 1)) {
			return _curCP = utf8::InvalidCP;
		}
	}
	uint8_t *p = buf;
	uint32_t cp = utf8::read_cp(p, len);
	if (cp == utf8::InvalidCP)
		return _curCP = utf8::InvalidCP;

	if (_curCP == '\n') {
		++_error._location._line;
		_error._location._posOnLine = 0;
	} else if (_curCP == '\r') {
		_error._location._posOnLine = 0;
	} else {
		++_error._location._posOnLine;
	}

	_curCP = cp;
	return _curCP;
}

bool Tokenizer::IsSeparator(uint32_t cp) const
{
	auto it = std::lower_bound(_separatorCPs.begin(), _separatorCPs.end(), cp);
	return it != _separatorCPs.end() && *it == cp;
}

bool Tokenizer::IsWhitespace(uint32_t cp)
{
	return cp == ' ' || cp == '\t' || cp == '\n' || cp == '\r';
}

int32_t Tokenizer::GetNumberValue(uint32_t cp, uint8_t base)
{
	int32_t diff = cp - '0';
	if (0 <= diff && diff < std::min<uint8_t>(base, 10))
		return diff;
	if ('a' <= cp && cp <= 'z')
		cp += 'A' - 'a';
	diff = cp - 'A';
	if (0 <= diff && diff < int32_t(base) - 10)
		return diff + 10;
	return -1;
}

bool Tokenizer::IsNumber(uint32_t cp, uint8_t base)
{
	return GetNumberValue(cp, base) >= 0;
}

bool Tokenizer::IsLetter(uint32_t cp)
{
	return 'a' <= cp && cp <= 'z' || 'A' <= cp && cp <= 'Z' || cp == '_';
}

void Tokenizer::AppendCP(String &s, uint32_t cp)
{
	uint8_t len = utf8::cp_size(cp);
	ASSERT(len > 0);
	s.resize(s.size() + len);
	uint8_t *p = (uint8_t *)(&s[s.size() - len]);
	utf8::write_cp(p, cp, len);
}

}
