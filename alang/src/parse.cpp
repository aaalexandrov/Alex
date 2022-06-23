#include "parse.h"
#include "dbg.h"

#include <unordered_map>
#include <unordered_set>

namespace alang {

ParseRule::Match::Match(Token::Class cls, String str, MatchRepeat rep)
	: _repeat{ rep }
	, _content{ Terminal{cls, str} }
{
}

ParseRule::Match::Match(String subruleId, MatchRepeat rep)
	: _repeat{ rep }
	, _content{ RuleRef{ subruleId } }
{
}

ParseRule::ParseRule(String id, std::initializer_list<Match> matches, MatchCombine combine)
	: _id{ id }
	, _combine{ combine }
	, _matches{ matches }
{
}

Parser::Parser(std::vector<ParseRule> const &rules)
	: _rules{ rules }
{
}

auto Parser::Parse(Tokenizer &tokens) const -> std::unique_ptr<Node>
{
	tokens.MoveNext();
	std::unique_ptr<Node> node = MatchRule(tokens, _rules[0]);
	bool atEof = tokens.Current()._type == Token::Type::Invalid && tokens.Current()._str.empty();
	if (atEof)
		return node;
	return nullptr;
}

void Parser::Dump(Node const *node, int32_t indent) const
{
	if (!node)
		return;
	std::cout << std::string(indent, ' ') << node->_rule->_id << std::endl;
	indent += 2;
	for (int32_t i = 0; i < node->GetContentSize(); ++i) {
		if (auto *token = node->GetToken(i)) {
			std::cout << std::string(indent, ' ') << token->Dump() << std::endl;
		} else {
			auto *sub = node->GetSubnode(i);
			Dump(sub, indent);
		}
	}
}

auto Parser::MatchRule(Tokenizer &tokens, ParseRule const &rule) const -> std::unique_ptr<Node>
{
	std::unique_ptr<Node> node;

	auto getNode = [&]()->Node * {
		if (!node)
			node = std::make_unique<Node>(&rule);
		return node.get();
	};

	for (auto &match : rule._matches) {

		bool matchFound;
		int32_t numMatches = 0;
		do {
			if (ParseRule::Terminal const *terminal = match.GetTerminal()) {
				matchFound = tokens.Current().GetClass() == terminal->_class && (terminal->_str.empty() || tokens.Current()._str == terminal->_str);
				if (matchFound) {
					getNode()->_content.push_back(tokens.Current());
					tokens.MoveNext();
				}
			} else {
				ParseRule *sub = match.GetSubrule()->_rule;
				std::unique_ptr<Node> subNode = MatchRule(tokens, *sub);
				matchFound = bool(subNode);
				if (matchFound) {
					getNode()->_content.push_back(std::move(subNode));
				}
			}
			numMatches += matchFound;
		} while (matchFound && match._repeat == MatchRepeat::ZeroMany);

		if (numMatches > 0 && rule._combine == MatchCombine::Alternative)
			return node;
		if (numMatches == 0 && rule._combine == MatchCombine::Sequence && match._repeat == MatchRepeat::One)
			return nullptr;
	}

	if (rule._combine == MatchCombine::Alternative)
		return nullptr;

	return node;
}

ParseRulesHolder::ParseRulesHolder(std::initializer_list<ParseRule> rules)
		: _rules(rules)
{
	// resolve references
	std::unordered_map<String, int32_t> id2ind;
	for (int32_t i = 0; i < _rules.size(); ++i) {
		id2ind.insert({ _rules[i]._id, i });
	}

	for (auto &rule : _rules) {
		for (auto &match : rule._matches) {
			ParseRule::RuleRef *sub = const_cast<ParseRule::RuleRef *>(match.GetSubrule());
			if (!sub)
				continue;
			auto it = id2ind.find(sub->_id);
			ASSERT(it != id2ind.end());
			sub->_rule = &_rules[it->second];
		}
	}
}

std::vector<String> ParseRulesHolder::GetKeyStrings() const
{
	std::unordered_set<String> uniqueKeys;
	for (auto &rule : _rules) {
		for (auto &match : rule._matches) {
			ParseRule::Terminal const *terminal = match.GetTerminal();
			if (!terminal || terminal->_class != Token::Class::Key)
				continue;
			ASSERT(!terminal->_str.empty());
			uniqueKeys.insert(terminal->_str);
		}
	}
	std::vector<String> keys(uniqueKeys.begin(), uniqueKeys.end());
	return keys;
}

ParseRulesHolder const &AlangRules()
{
	static ParseRulesHolder s_langRules{
		{"MODULE", {{Token::Class::Key, "module"}, {Token::Class::Identifier}, {"DEFINITION", MatchRepeat::ZeroMany}, {Token::Class::Key, "end"}} },

		{"DEFINITION", {{"DEF_VALUE"}, {"DEF_FUNC"}, {"IMPORT"}, {"MODULE"}}, MatchCombine::Alternative},

		{"QUALIFIED_NAME", {{Token::Class::Identifier}, {"DOT_IDENTIFIER", MatchRepeat::ZeroMany}}},
		{"DOT_IDENTIFIER", {{Token::Class::Key, "."}, {Token::Class::Identifier}}},

		{"IMPORT", {{Token::Class::Key, "import"},  {"QUALIFIED_NAME"}, {"IMPORT_TAIL", MatchRepeat::ZeroMany}}},
		{"IMPORT_TAIL", {{Token::Class::Key, ","}, {"QUALIFIED_NAME"}}},

		{"DEF_VALUE", {{"CONST_OR_VAR"}, {Token::Class::Identifier}, {"OF_TYPE"}, {"EQ_EXPRESSION", MatchRepeat::ZeroOne}}},
		{"OF_TYPE", {{Token::Class::Key, ":"}, {"TYPE"}}},
		{"CONST_OR_VAR", {{Token::Class::Key, "const"}, {Token::Class::Key, "var"}}, MatchCombine::Alternative},
		{"EQ_EXPRESSION", {{Token::Class::Key, "="}, {"EXPRESSION"}}},

		{"TYPE", {{"QUALIFIED_NAME"}}},

		{"DEF_FUNC", {{Token::Class::Key, "func"}, {Token::Class::Identifier}, {Token::Class::Key, "("}, {"DEF_PARAM_LIST", MatchRepeat::ZeroOne}, {Token::Class::Key, ")"}, {"OF_TYPE", MatchRepeat::ZeroOne}, {"OPERATOR", MatchRepeat::ZeroMany}, {Token::Class::Key, "end"}}},
		{"DEF_PARAM_LIST", {{"DEF_PARAM"}, {"DEF_PARAM_TAIL", MatchRepeat::ZeroMany}}},
		{"DEF_PARAM", {{Token::Class::Identifier}, {"OF_TYPE"}}},
		{"DEF_PARAM_TAIL", {{Token::Class::Key, ","}, {"DEF_PARAM"}}},

		{"OPERATOR", {{"DEF_VALUE"}, {"IF"}, {"WHILE"}, {"RETURN"}, {"ASSIGN_OR_CALL"}}, MatchCombine::Alternative},

		{"IF", {{Token::Class::Key, "if"}, {"EXPRESSION"}, {"OPERATOR", MatchRepeat::ZeroMany}, {"ELSE", MatchRepeat::ZeroOne}, {Token::Class::Key, "end"}}},
		{"ELSE", {{Token::Class::Key, "if"}, {"OPERATOR", MatchRepeat::ZeroMany}}},

		{"WHILE", {{Token::Class::Key, "while"}, {"EXPRESSION"}, {"OPERATOR", MatchRepeat::ZeroMany}, {Token::Class::Key, "end"}}},

		{"RETURN", {{Token::Class::Key, "return"}, {"EXPRESSION", MatchRepeat::ZeroOne}}},

		{"ASSIGN_OR_CALL", {{Token::Class::Identifier}, {"ASSIGN_OR_CALL_TAIL"}}},
		{"ASSIGN_OR_CALL_TAIL", {{"EQ_EXPRESSION"}, {"DOT_IDENT_ASSGN_TAIL"}, {"INDEX_ASSGN_TAIL"}, {"CALL_PARAMS_ASSGN_TAIL"}}, MatchCombine::Alternative},
		{"DOT_IDENT_ASSGN_TAIL", {{"DOT_IDENTIFIER"}, {"ASSIGN_OR_CALL_TAIL"}}},
		{"INDEX_ASSGN_TAIL", {{"INDEX"}, {"ASSIGN_OR_CALL_TAIL"}}},
		{"CALL_PARAMS_ASSGN_TAIL", {{"CALL_PARAMS"}, {"ASSIGN_OR_CALL_TAIL", MatchRepeat::ZeroOne}}},

		{"EXPRESSION", {{"COMPARISON"}}},

		{"COMPARISON", {{"NEGATION"}, {"CMP_NEGATION", MatchRepeat::ZeroOne}}},
		{"NEGATION", {{"NOT_VALUE"}, {"ADDITION"}}, MatchCombine::Alternative},
		{"NOT_VALUE", {{Token::Class::Key, "!"}, {"VALUE"}}},
		{"CMP_NEGATION", {{"CMP"}, {"NEGATION"}}},
		{"CMP", {{Token::Class::Key, "=="}, {Token::Class::Key, "!="}, {Token::Class::Key, "<"}, {Token::Class::Key, ">"}, {Token::Class::Key, "<="}, {Token::Class::Key, ">="}}, MatchCombine::Alternative},

		{"ADDITION", {{"PLUS_MINUS", MatchRepeat::ZeroOne}, {"MULTIPLICATION"}, {"ADDITION_TAIL", MatchRepeat::ZeroMany}}},
		{"ADDITION_TAIL", {{"PLUS_MINUS"}, {"MULTIPLICATION"}}},
		{"PLUS_MINUS", {{Token::Class::Key, "+"}, {Token::Class::Key, "-"}}, MatchCombine::Alternative},

		{"MULTIPLICATION", {{"POWER"}, {"MULTIPLICATION_TAIL", MatchRepeat::ZeroMany}}},
		{"MULTIPLICATION_TAIL", {{"MUL_DIV_REM"}, {"POWER"}}},
		{"MUL_DIV_REM", {{Token::Class::Key, "*"}, {Token::Class::Key, "/"}, {Token::Class::Key, "%"}}, MatchCombine::Alternative},

		{"POWER", {{"VALUE"}, {"POWER_TAIL", MatchRepeat::ZeroMany}}},
		{"POWER_TAIL", {{Token::Class::Key, "^"}, {"VALUE"}}},

		{"VALUE", {{Token::Class::Literal}, {"EXPR_IN_PAREN"}, {"VAR_CALL_INDEXED"}}, MatchCombine::Alternative},
		{"EXPR_IN_PAREN", {{Token::Class::Key, "("}, {"EXPRESSION"}, {Token::Class::Key, ")"}}},
		{"VAR_CALL_INDEXED", {{Token::Class::Identifier}, {"VALUE_QUALIFIERS", MatchRepeat::ZeroMany}}},
		{"VALUE_QUALIFIERS", {{"DOT_IDENTIFIER"}, {"INDEX"}, {"CALL_PARAMS"}}, MatchCombine::Alternative},
		{"INDEX", {{Token::Class::Key, "["}, {"EXPRESSION_LIST"}, {Token::Class::Key, "]"}}},
		{"CALL_PARAMS", {{Token::Class::Key, "("}, {"EXPRESSION_LIST"}, {Token::Class::Key, ")"}}},

		{"EXPRESSION_LIST", {{"EXPRESSION"}, {"COMMA_EXPRESSION", MatchRepeat::ZeroMany}}},
		{"COMMA_EXPRESSION", {{Token::Class::Key, ","}, {"EXPRESSION"}}},
	};

	return s_langRules;
}

}