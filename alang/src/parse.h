#pragma once

#include "token.h"
#include <variant>

namespace alang {

struct ParseRule {
	enum class CombineMode {
		Sequence,
		Alternative,
	};

	enum class Repeat {
		One,
		ZeroOne,
		ZeroMany,
	};

	struct Terminal {
		Token::Class _class = Token::Class::Invalid;
		String _str;
	};

	struct RuleRef {
		String _id;
		ParseRule *_rule = nullptr;
	};

	struct Match {
		Repeat _repeat = Repeat::One;
		std::variant<Terminal, RuleRef> _content;

		Match(Token::Class cls, String str = "", Repeat rep = Repeat::One);
		Match(String subruleId, Repeat rep = Repeat::One);

		Terminal const *GetTerminal() const { return std::get_if<Terminal>(&_content); }
		RuleRef const *GetSubrule() const { return std::get_if<RuleRef>(&_content); }
	};

	String _id;
	CombineMode _combine = CombineMode::Sequence;
	std::vector<Match> _matches;

	ParseRule(String id, std::initializer_list<Match> matches, CombineMode combine = CombineMode::Sequence);
};

struct Parser {
	struct Node {
		using Content = std::variant<Token, std::unique_ptr<Node>>;

		ParseRule const *_rule;
		std::vector<Content> _content;

		Node(ParseRule const *rule) : _rule(rule) {}

		int32_t GetContentSize() const { return int32_t(_content.size()); }
		Token const *GetToken(int32_t i) const { return std::get_if<Token>(&_content[i]); }
		Node const *GetSubnode(int32_t i) const { return std::holds_alternative<std::unique_ptr<Node>>(_content[i]) ? std::get<std::unique_ptr<Node>>(_content[i]).get() : nullptr; }
	};

	std::vector<ParseRule> _rules;

	Parser(std::vector<ParseRule> &&rules);

	std::vector<String> GetKeyStrings() const;

	std::unique_ptr<Node> Parse(Tokenizer &tokens) const;

	void Dump(Node const *node, int32_t indent = 0) const;

protected:
	std::unique_ptr<Node> MatchRule(Tokenizer &tokens, ParseRule const &rule) const;
};

std::unique_ptr<Parser> GetParser();

}