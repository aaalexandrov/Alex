#pragma once

#include <fstream>
#include "utf8.h"
#include "common.h"

namespace alang {

struct Token {
	enum class Type {
		Invalid,
		Keyword,
		Separator,
		Identifier,
		String,
		Char,
		Integer,
		Double,
	};

	enum class Class {
		Invalid,
		Key,
		Identifier,
		Literal,
	};

	Type _type = Type::Invalid;
	String _str;
	union {
		uint64_t _integer = 0;
		double _double;
	};
	PosInFile _filePos;

	Class GetClass() const;
	std::string Dump() const;
};

struct Tokenizer {
	Tokenizer(std::string filePath, std::vector<String> const &keywordsSeparators);

	std::string const &GetFilePath() const { return _filePath; }
	std::streamsize GetFileSize() const { return _fileSize; }
	PosInFile GetPosInFile() const { return _filePos; }

	Token const &Current() const;
	bool MoveNext();

	String GetError() const;

protected:
	void InitKeywordsSeparators(std::vector<String> const &keywordsSeparators);

	bool ParseToken(Token &token);

	bool ParseIdentifier(Token &token);
	bool ParseSeparator(Token &token);
	bool ParseString(Token &token);
	bool ParseChar(Token &token);
	bool ParseNumber(Token &token);

	bool SkipWhitespace();
	bool SkipComment();

	uint32_t NextCP();

	bool IsSeparator(uint32_t cp) const;
	static bool IsWhitespace(uint32_t cp);
	static int32_t GetNumberValue(uint32_t cp, uint8_t base = 10);
	static bool IsNumber(uint32_t cp, uint8_t base = 10);
	static bool IsLetter(uint32_t cp);

	static void AppendCP(String &s, uint32_t cp);

	static inline constexpr uint32_t s_bufferSize = 32768;

	std::vector<String> _keywords;
	std::vector<String> _separators;
	std::vector<uint32_t> _separatorCPs;

	Token _curToken;

	uint32_t _curCP = utf8::InvalidCP;
	PosInFile _filePos{0, 0};

	String _filePath;
	std::ifstream _file;
	std::streamsize _fileSize = 0;
	std::vector<uint8_t> _buffer;
};

}