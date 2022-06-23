#pragma once

#include "token.h"
#include <variant>

namespace alang {

enum class MatchCombine {
	Sequence,
	Alternative,
};

enum class MatchRepeat {
	One,
	ZeroOne,
	ZeroMany,
};

struct ParseRule {
	struct Terminal {
		Token::Class _class = Token::Class::Invalid;
		String _str;
	};

	struct RuleRef {
		String _id;
		ParseRule *_rule = nullptr;
	};

	struct Match {
		MatchRepeat _repeat = MatchRepeat::One;
		std::variant<Terminal, RuleRef> _content;

		Match(Token::Class cls, String str = "", MatchRepeat rep = MatchRepeat::One);
		Match(String subruleId, MatchRepeat rep = MatchRepeat::One);

		Terminal const *GetTerminal() const { return std::get_if<Terminal>(&_content); }
		RuleRef const *GetSubrule() const { return std::get_if<RuleRef>(&_content); }
	};

	String _id;
	MatchCombine _combine = MatchCombine::Sequence;
	std::vector<Match> _matches;

	ParseRule(String id, std::initializer_list<Match> matches, MatchCombine combine = MatchCombine::Sequence);
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

	std::vector<ParseRule> const &_rules;

	Parser(std::vector<ParseRule> const &rules);

	std::unique_ptr<Node> Parse(Tokenizer &tokens) const;

	void Dump(Node const *node, int32_t indent = 0) const;

protected:
	std::unique_ptr<Node> MatchRule(Tokenizer &tokens, ParseRule const &rule) const;
};

struct ParseRulesHolder {
	std::vector<ParseRule> _rules;

	ParseRulesHolder(std::initializer_list<ParseRule> rules);
	std::vector<String> GetKeyStrings() const;
};

ParseRulesHolder const &AlangRules();

}