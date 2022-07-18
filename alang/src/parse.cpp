#include "parse.h"
#include "dbg.h"

#include <unordered_map>
#include <unordered_set>

namespace alang {

ParseRule::Match::Match(Token::Class cls, String str, MatchOptions opt)
	: _opt{ opt }
	, _content{ Terminal{cls, str} }
{
	if (_opt._output == Output::Auto)
		_opt._output = (cls == Token::Class::Key) ? Output::Disable : Output::Enable;
}

ParseRule::Match::Match(String subruleId, MatchOptions opt)
	: _opt{ opt }
	, _content{ RuleRef{ subruleId } }
{
	if (_opt._output == Output::Auto)
		_opt._output = Output::Enable;
}

ParseRule::ParseRule(String id, std::initializer_list<Match> matches, ParseOptions opt)
	: _id{ id }
	, _opt{ opt }
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
	auto &rootRule = _rules[0];
	auto node = std::make_unique<Node>(&rootRule);
	bool match = MatchRule(tokens, rootRule, node);
	bool atEof = tokens.Current()._type == Token::Type::Invalid && tokens.Current()._str.empty();
	if (match && atEof)
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

bool Parser::MatchRule(Tokenizer &tokens, ParseRule const &rule, std::unique_ptr<Node> &node) const
{
	bool anyMatch = false;
	for (auto &match : rule._matches) {
		bool matchFound;
		int32_t numMatches = 0;
		do {
			if (ParseRule::Terminal const *terminal = match.GetTerminal()) {
				matchFound = tokens.Current().GetClass() == terminal->_class && (terminal->_str.empty() || tokens.Current()._str == terminal->_str);
				if (matchFound) {
					if (match._opt._output == Output::Enable) {
						node->_content.push_back(tokens.Current());
					}
					tokens.MoveNext();
				}
			} else {
				ParseRule *sub = match.GetSubrule()->_rule;
				size_t contentSize = node->_content.size();
				switch (sub->_opt._nodeOutput) {
					case NodeOutput::Own:
						node->_content.push_back(std::make_unique<Node>(sub));
						matchFound = MatchRule(tokens, *sub, std::get<std::unique_ptr<Node>>(node->_content.back()));
						break;
					case NodeOutput::Parent:
						matchFound = MatchRule(tokens, *sub, node);
						break;
					case NodeOutput::ReplaceInParent: {
						auto subNode = std::make_unique<Node>(sub);
						if (contentSize) {
							subNode->_content.push_back(std::move(node->_content.back()));
							node->_content.resize(contentSize - 1);
						}
						node->_content.push_back(std::move(subNode));
						matchFound = MatchRule(tokens, *sub, std::get<std::unique_ptr<Node>>(node->_content.back()));
					}
						break;
					default:
						ASSERT(0);
						matchFound = false;
				}
				if (!matchFound) {
					if (sub->_opt._nodeOutput == NodeOutput::ReplaceInParent) {
						ASSERT(node->_content.size() == contentSize);
						auto subNode = std::move(std::get<std::unique_ptr<Node>>(node->_content.back()));
						node->_content.back() = std::move(subNode->_content.front());
					} else {
						node->_content.resize(contentSize);
					}
				}
			}
			numMatches += matchFound;
			anyMatch |= matchFound;
		} while (matchFound && match._opt._repeat == Repeat::ZeroMany);

		if (numMatches > 0 && rule._opt._combine == Combine::Alternative)
			break;
		if (numMatches == 0 && rule._opt._combine == Combine::Sequence && match._opt._repeat == Repeat::One)
			return false;
	}

	if (rule._opt._rename == Rename::Disable && node && node->_content.size() == 1 && node->GetSubnode(0)) {
		node = std::move(std::get<std::unique_ptr<Node>>(node->_content[0]));
	}

	return anyMatch;
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
		{"MODULE", {{Token::Class::Key, "module"}, {Token::Class::Identifier}, {"DEFINITION", Repeat::ZeroMany}, {Token::Class::Key, "end"}} },

		{"DEFINITION", {{"DEF_VALUE"}, {"DEF_FUNC"}, {"IMPORT"}, {"MODULE"}}, {Combine::Alternative, Rename::Disable}},

		{"QUALIFIED_NAME", {{Token::Class::Identifier}, {"SUBSCRIPTS", Repeat::ZeroOne}}, NodeOutput::Parent},
		{"SUBSCRIPTS", {{"SUBSCRIPT", Repeat::ZeroMany}}, NodeOutput::ReplaceInParent},
		{"SUBSCRIPT", {{Token::Class::Key, "."}, {Token::Class::Identifier}}, NodeOutput::Parent},

		{"IMPORT", {{Token::Class::Key, "import"},  {"QUALIFIED_NAME"}, {"IMPORT_TAIL", Repeat::ZeroMany}}},
		{"IMPORT_TAIL", {{Token::Class::Key, ","}, {"QUALIFIED_NAME"}}},

		{"DEF_VALUE", {{"CONST_OR_VAR"}, {Token::Class::Identifier}, {"OF_TYPE"}, {"EQ_EXPRESSION", Repeat::ZeroOne}}},
		{"OF_TYPE", {{Token::Class::Key, ":"}, {"TYPE"}}, NodeOutput::Parent},
		{"CONST_OR_VAR", {{Token::Class::Key, "const", Output::Enable}, {Token::Class::Key, "var", Output::Enable}}, {Combine::Alternative, NodeOutput::Parent}},
		{"EQ_EXPRESSION", {{Token::Class::Key, "="}, {"EXPRESSION"}}, NodeOutput::ReplaceInParent},

		{"TYPE", {{"QUALIFIED_NAME"}, {"GENERIC_PARAMS", Repeat::ZeroOne}}},
		{"GENERIC_PARAMS", {{Token::Class::Key, "{"}, {"EXPRESSION_LIST"}, {Token::Class::Key, "}"}}},

		{"DEF_FUNC", {{Token::Class::Key, "func"}, {Token::Class::Identifier}, {Token::Class::Key, "("}, {"DEF_PARAM_LIST", Repeat::ZeroOne}, {Token::Class::Key, ")"}, {"OF_TYPE", Repeat::ZeroOne}, {"OPERATOR", Repeat::ZeroMany}, {Token::Class::Key, "end"}}},
		{"DEF_PARAM_LIST", {{"DEF_PARAM"}, {"DEF_PARAM_TAIL", Repeat::ZeroMany}}},
		{"DEF_PARAM", {{Token::Class::Identifier}, {"OF_TYPE"}}},
		{"DEF_PARAM_TAIL", {{Token::Class::Key, ","}, {"DEF_PARAM"}}},

		{"OPERATOR", {{"DEF_VALUE"}, {"IF"}, {"WHILE"}, {"RETURN"}, {"ASSIGN_OR_CALL"}}, {Combine::Alternative, Rename::Disable}},

		{"IF", {{Token::Class::Key, "if"}, {"EXPRESSION"}, {"OPERATOR", Repeat::ZeroMany}, {"ELSE", Repeat::ZeroOne}, {Token::Class::Key, "end"}}},
		{"ELSE", {{Token::Class::Key, "if"}, {"OPERATOR", Repeat::ZeroMany}}},

		{"WHILE", {{Token::Class::Key, "while"}, {"EXPRESSION"}, {"OPERATOR", Repeat::ZeroMany}, {Token::Class::Key, "end"}}},

		{"RETURN", {{Token::Class::Key, "return"}, {"EXPRESSION", Repeat::ZeroOne}}},

		{"ASSIGN_OR_CALL", {{"VALUE_BASE"}, {"ASSIGN_OR_CALL_TAIL"}}, Rename::Disable},
		{"ASSIGN_OR_CALL_TAIL", {{"EQ_EXPRESSION"}, {"DOT_IDENT_ASSGN_TAIL"}, {"INDEX_ASSGN_TAIL"}, {"CALL_ASSGN_TAIL"}}, {Combine::Alternative, NodeOutput::Parent}},
		{"DOT_IDENT_ASSGN_TAIL", {{"SUBSCRIPTS"}, {"ASSIGN_OR_CALL_TAIL"}}, NodeOutput::Parent},
		{"INDEX_ASSGN_TAIL", {{"INDEX"}, {"ASSIGN_OR_CALL_TAIL"}}, NodeOutput::Parent},
		{"CALL_ASSGN_TAIL", {{"CALL"}, {"ASSIGN_OR_CALL_TAIL", Repeat::ZeroOne}}, NodeOutput::Parent},

		{"EXPRESSION", {{"COMPARISON"}}, Rename::Disable},

		{"COMPARISON", {{"NEGATION"}, {"CMP_NEGATION", Repeat::ZeroOne}}, Rename::Disable},
		{"NEGATION", {{"NOT_VALUE"}, {"ADDITION"}}, {Combine::Alternative, Rename::Disable}},
		{"NOT_VALUE", {{Token::Class::Key, "!"}, {"VALUE"}}},
		{"CMP_NEGATION", {{"CMP"}, {"NEGATION"}}, NodeOutput::Parent},
		{"CMP", {{Token::Class::Key, "==", Output::Enable}, {Token::Class::Key, "!=", Output::Enable}, {Token::Class::Key, "<", Output::Enable}, {Token::Class::Key, ">", Output::Enable}, {Token::Class::Key, "<=", Output::Enable}, {Token::Class::Key, ">=", Output::Enable}}, {Combine::Alternative, NodeOutput::Parent}},

		{"ADDITION", {{"MULTIPLICATION"}, {"ADDITION_TAIL", Repeat::ZeroMany}}, Rename::Disable},
		{"ADDITION_TAIL", {{"PLUS_MINUS"}, {"MULTIPLICATION"}}, NodeOutput::Parent},
		{"PLUS_MINUS", {{Token::Class::Key, "+", Output::Enable}, {Token::Class::Key, "-", Output::Enable}}, {Combine::Alternative, NodeOutput::Parent}},

		{"MULTIPLICATION", {{"POWER"}, {"MULTIPLICATION_TAIL", Repeat::ZeroMany}}, Rename::Disable},
		{"MULTIPLICATION_TAIL", {{"MUL_DIV_REM"}, {"POWER"}}, NodeOutput::Parent},
		{"MUL_DIV_REM", {{Token::Class::Key, "*", Output::Enable}, {Token::Class::Key, "/", Output::Enable}, {Token::Class::Key, "%", Output::Enable}}, {Combine::Alternative, NodeOutput::Parent}},

		{"POWER", {{"SIGNED"}, {"POWER_TAIL", Repeat::ZeroMany}}, Rename::Disable},
		{"POWER_TAIL", {{Token::Class::Key, "^"}, {"SIGNED"}}, NodeOutput::Parent},

		{"SIGNED", {{"PLUS_MINUS", Repeat::ZeroOne}, {"VALUE"}}, Rename::Disable},

		{"VALUE", {{Token::Class::Literal}, {"VAR_CALL_INDEXED"}}, Combine::Alternative},
		{"VALUE_BASE", {{Token::Class::Key, "&", {Repeat::ZeroMany, Output::Enable}}, {"SUBEXPR_OR_IDENT"}}, NodeOutput::Parent},
		{"SUBEXPR_OR_IDENT", {{"EXPR_IN_PAREN"}, {Token::Class::Identifier}}, {Combine::Alternative, NodeOutput::Parent}},
		{"EXPR_IN_PAREN", {{Token::Class::Key, "("}, {"EXPRESSION"}, {Token::Class::Key, ")"}}, NodeOutput::Parent},
		{"VAR_CALL_INDEXED", {{"VALUE_BASE"}, {"VALUE_QUALIFIERS", Repeat::ZeroMany}}, NodeOutput::Parent},
		{"VALUE_QUALIFIERS", {{"SUBSCRIPT"}, {"INDEX"}, {"CALL"}}, {Combine::Alternative, NodeOutput::Parent}},
		{"INDEX", {{Token::Class::Key, "["}, {"EXPRESSION_LIST"}, {Token::Class::Key, "]"}}, NodeOutput::ReplaceInParent},
		{"CALL", {{Token::Class::Key, "("}, {"EXPRESSION_LIST"}, {Token::Class::Key, ")"}}, NodeOutput::ReplaceInParent},

		{"EXPRESSION_LIST", {{"EXPRESSION"}, {"COMMA_EXPRESSION", Repeat::ZeroMany}}, NodeOutput::Parent},
		{"COMMA_EXPRESSION", {{Token::Class::Key, ","}, {"EXPRESSION"}}, NodeOutput::Parent},
	};

	return s_langRules;
}

}