#include "alang.h"
#include "dbg.h"
#include "utf8.h"
#include "token.h"
#include "parse.h"
#include "analyze.h"

#include <vector>
#include <fstream>
#include <filesystem>

using namespace std;

void TestUtf8()
{
	std::ifstream file("test/utf8test.txt", std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	ASSERT(size > 0);

	std::vector<uint8_t> buffer(size);
	if (file.read((char *)buffer.data(), size)) {
		std::vector<uint32_t> str;
		uint8_t *p = buffer.data(), *end = buffer.data() + buffer.size();
		while (p < end) {
			uint8_t *prev = p;
			uint32_t cp = utf8::read_cp(p);
			ASSERT(p - prev == utf8::cp_size(cp));
			ASSERT(utf8::cp_size(prev) == utf8::cp_size(cp));
			if (cp == utf8::InvalidCP || cp == 0) {
				ASSERT(0);
				break;
			}
			str.push_back(cp);
		}
		std::vector<uint8_t> s;
		for (uint32_t cp : str) {
			uint8_t len = utf8::cp_size(cp);
			ASSERT(len > 0);
			s.resize(s.size() + len);
			uint8_t *p = &s[s.size() - len];
			uint8_t l = utf8::write_cp(p, cp, len);
			ASSERT(l == len);
			ASSERT(p == s.data() + s.size());
		}
		ASSERT(buffer.size() == s.size());
		for (size_t i = 0; i < buffer.size(); ++i) {
			ASSERT(buffer[i] == s[i]);
		}
	}
}

void ParseFile(std::string path)
{
	auto &rules = alang::AlangRules();
	alang::Tokenizer tokens(path, rules.GetKeyStrings());
	auto parser = std::make_unique<alang::Parser>(rules._rules);
	cout << "File: " << tokens.GetFilePath() << " size: " << tokens.GetFileSize() << endl;
	auto parsed = parser->Parse(tokens);
	if (parsed) {
		parser->Dump(parsed.get());
	}
	if (!tokens.GetError().empty()) {
		cout << "Parsing error: " << tokens.GetError() << " at pos " << tokens.GetPosInFile()._line << ":" << tokens.GetPosInFile()._posOnLine << endl;
		return;
	}

	alang::Analyzer analyzer;
	auto error = analyzer.AnalyzeDefinitions(parsed.get(), nullptr);
	if (!error._error.empty()) {
		alang::PosInFile errPos = error._location->_filePos;
		cout << "Semantic error: " << error._error << " at pos " << errPos._line << ":" << errPos._posOnLine << endl;
	}
}

int main()
{
	cout << std::filesystem::current_path() << endl;

	//TestUtf8();

	ParseFile("test/parse_test.al");

	return 0;
}
