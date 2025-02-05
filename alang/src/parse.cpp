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

auto Parser::Parse(Tokenizer &tokens) const -> std::unique_ptr<ParseNode>
{
	tokens.MoveNext();
	auto &rootRule = _rules[0];
	auto node = std::make_unique<ParseNode>(&rootRule, tokens.Current()._filePos);
	bool match = MatchRule(tokens, rootRule, node);
	bool atEof = tokens.Current()._type == Token::Type::Invalid && tokens.Current()._str.empty();
	if (match && atEof)
		return node;
	return nullptr;
}

void Parser::Dump(ParseNode const *node, int32_t indent) const
{
	if (!node)
		return;
	std::cout << std::string(indent, ' ') << node->_label << " (" << node->_rule->_id << ")" << std::endl;
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

bool Parser::MatchRule(Tokenizer &tokens, ParseRule const &rule, std::unique_ptr<ParseNode> &node) const
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
						node->_content.emplace_back(tokens.Current());
					}
					if (node->_label.empty() && tokens.Current().GetClass() == Token::Class::Key) {
						node->_label = tokens.Current()._str;
					}

					tokens.MoveNext();
				}
			} else {
				ParseRule *sub = match.GetSubrule()->_rule;
				size_t contentSize = node->_content.size();
				switch (sub->_opt._nodeOutput) {
					case NodeOutput::Own:
						node->_content.push_back(std::make_unique<ParseNode>(sub, tokens.Current()._filePos));
						matchFound = MatchRule(tokens, *sub, std::get<std::unique_ptr<ParseNode>>(node->_content.back()._tokenOrNode));
						break;
					case NodeOutput::Parent:
						matchFound = MatchRule(tokens, *sub, node);
						break;
					case NodeOutput::ReplaceInParent: {
						auto subNode = std::make_unique<ParseNode>(sub, tokens.Current()._filePos);
						if (contentSize) {
							subNode->_content.push_back(std::move(node->_content.back()));
							node->_content.resize(contentSize - 1);
						}
						node->_content.push_back(std::move(subNode));
						matchFound = MatchRule(tokens, *sub, std::get<std::unique_ptr<ParseNode>>(node->_content.back()._tokenOrNode));
					}
						break;
					default:
						ASSERT(0);
						matchFound = false;
				}
				if (!matchFound) {
					if (sub->_opt._nodeOutput == NodeOutput::ReplaceInParent) {
						ASSERT(node->_content.size() == contentSize);
						auto subNode = std::move(std::get<std::unique_ptr<ParseNode>>(node->_content.back()._tokenOrNode));
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
		node = std::move(std::get<std::unique_ptr<ParseNode>>(node->_content[0]._tokenOrNode));
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
		{"MODULE", {{Token::Class::Key, "module"}, {"QUALIFIED_NAME"}, {"DEFINITION", Repeat::ZeroMany}, {Token::Class::Key, "end"}}},

		{"GENERIC_PARAM", {{Token::Class::Literal}, {"QUALIFIED_NAME"}}, {Combine::Alternative, NodeOutput::Parent, Rename::Disable}},
		{"GENERIC_PARAM_TAIL", {{Token::Class::Key, ","}, {"GENERIC_PARAM"}}, NodeOutput::Parent},
		{"GENERIC_PARAMS", {{Token::Class::Key, "{"}, {"GENERIC_PARAM"}, {"GENERIC_PARAM_TAIL", Repeat::ZeroMany}, {Token::Class::Key, "}"}}, NodeOutput::ReplaceInParent},
		{"UNQUALIFIED_NAME", {{Token::Class::Identifier}, {"GENERIC_PARAMS", Repeat::ZeroOne}}, NodeOutput::Parent},
		{"UNQUALIFIED_NAME_TAIL", {{Token::Class::Key, "."}, {"UNQUALIFIED_NAME"}}, NodeOutput::Parent},
		{"UNQUALIFIED_NAME_SUBSCRIPTS", {{"UNQUALIFIED_NAME_TAIL", Repeat::ZeroMany}}, NodeOutput::ReplaceInParent},
		{"QUALIFIED_NAME", {{"UNQUALIFIED_NAME"}, {"UNQUALIFIED_NAME_SUBSCRIPTS", Repeat::ZeroOne}}, NodeOutput::Parent},
		{"QUALIFIED_NAME_TAIL", {{Token::Class::Key, ","}, {"QUALIFIED_NAME"}}, NodeOutput::Parent},

		{"DEFINITION", {{"DEF_VALUE"}, {"DEF_FUNC"}, {"IMPORT"}, {"MODULE"}}, {Combine::Alternative, Rename::Disable}},

		{"IMPORT", {{Token::Class::Key, "import"},  {"QUALIFIED_NAME"}, {"QUALIFIED_NAME_TAIL", Repeat::ZeroMany}}},

		{"DEF_VALUE", {{"DEF_CONST"}, {"DEF_VAR"}}, {Combine::Alternative, Rename::Disable}},
		{"DEF_CONST", {{Token::Class::Key, "const"}, {Token::Class::Identifier}, {"OF_TYPE"}, {"INITIALIZE"}}},
		{"DEF_VAR", {{Token::Class::Key, "var"}, {Token::Class::Identifier}, {"OF_TYPE"}, {"INITIALIZE", Repeat::ZeroOne}}},
		{"OF_TYPE", {{Token::Class::Key, ":"}, {"TYPE"}}/*, NodeOutput::ReplaceInParent*/},
		{"INITIALIZE", {{Token::Class::Key, "="}, {"EXPRESSION"}}/*, NodeOutput::ReplaceInParent*/},

		{"TYPE", {{"QUALIFIED_NAME"}}, NodeOutput::Parent},

		{"DEF_FUNC", {{Token::Class::Key, "func"}, {Token::Class::Identifier}, {Token::Class::Key, "("}, {"DEF_PARAM_LIST", Repeat::ZeroOne}, {Token::Class::Key, ")"}, {"RETURN_TYPE", Repeat::ZeroOne}, {"OPERATOR_LIST", Repeat::ZeroMany}, {Token::Class::Key, "end"}}},
		{"RETURN_TYPE", {{Token::Class::Key, ":"}, {"TYPE"}}},
		{"DEF_PARAM_LIST", {{"DEF_PARAM"}, {"DEF_PARAM_TAIL", Repeat::ZeroMany}}, NodeOutput::Parent},
		{"DEF_PARAM", {{Token::Class::Identifier}, {"OF_TYPE"}}, NodeOutput::Parent},
		{"DEF_PARAM_TAIL", {{Token::Class::Key, ","}, {"DEF_PARAM"}}, NodeOutput::Parent},

		{"OPERATOR", {{"DEF_VALUE"}, {"IF"}, {"WHILE"}, {"RETURN"}, {"ASSIGN_OR_CALL"}}, {Combine::Alternative, Rename::Disable}},
		{"OPERATOR_LIST", {{"OPERATOR", Repeat::ZeroMany}}},

		{"IF", {{Token::Class::Key, "if"}, {"EXPRESSION"}, {"OPERATOR", Repeat::ZeroMany}, {"ELSE", Repeat::ZeroOne}, {Token::Class::Key, "end"}}},
		{"ELSE", {{Token::Class::Key, "if"}, {"OPERATOR", Repeat::ZeroMany}}},

		{"WHILE", {{Token::Class::Key, "while"}, {"EXPRESSION"}, {"OPERATOR", Repeat::ZeroMany}, {Token::Class::Key, "end"}}},

		{"RETURN", {{Token::Class::Key, "return"}, {"EXPRESSION", Repeat::ZeroOne}}},

		{"ASSIGN", {{Token::Class::Key, "="}, {"EXPRESSION"}}, NodeOutput::ReplaceInParent},
		{"ASSIGN_OR_CALL", {{"VALUE_BASE"}, {"ASSIGN_OR_CALL_TAIL"}}, Rename::Disable},
		{"ASSIGN_OR_CALL_TAIL", {{"ASSIGN"}, {"DOT_IDENT_ASSGN_TAIL"}, {"INDEX_ASSGN_TAIL"}, {"CALL_ASSGN_TAIL"}}, {Combine::Alternative, NodeOutput::Parent}},
		{"DOT_IDENT_ASSGN_TAIL", {{"UNQUALIFIED_NAME_SUBSCRIPTS"}, {"ASSIGN_OR_CALL_TAIL"}}, NodeOutput::Parent},
		{"INDEX_ASSGN_TAIL", {{"INDEX"}, {"ASSIGN_OR_CALL_TAIL"}}, NodeOutput::Parent},
		{"CALL_ASSGN_TAIL", {{"CALL"}, {"ASSIGN_OR_CALL_TAIL", Repeat::ZeroOne}}, NodeOutput::Parent},

		{"EXPRESSION", {{"LOGIC_OP"}}, NodeOutput::Parent},

		{"LOGIC_OP", {{"COMPARISON"}, {"AND_OR", Repeat::ZeroMany}}, NodeOutput::Parent},
		{"AND_OR", {{"ANDS"}, {"ORS"}}, {Combine::Alternative, NodeOutput::Parent}},
		{"ANDS", {{"AND", Repeat::ZeroMany}}, NodeOutput::ReplaceInParent},
		{"AND", {{Token::Class::Key, "&&"}, {"COMPARISON"}}, NodeOutput::Parent},
		{"ORS", {{"OR", Repeat::ZeroMany}}, NodeOutput::ReplaceInParent},
		{"OR", {{Token::Class::Key, "||"}, {"COMPARISON"}}, NodeOutput::Parent},

		{"COMPARISON", {{"CMP_ARG"}, {"CMP", Repeat::ZeroOne}}, NodeOutput::Parent},

		{"CMP_ARG", {{"NOT"}, {"ADDITION"}}, {Combine::Alternative, NodeOutput::Parent}},
		{"NOT", {{Token::Class::Key, "!"}, {"ADDITION"}}},

		{"CMP", {{"CMP_EQ"}, {"CMP_NEQ"}, {"CMP_LESS"}, {"CMP_LEQ"}, {"CMP_GT"}, {"CMP_GTEQ"}}, {Combine::Alternative, NodeOutput::Parent}},
		{"CMP_EQ", {{Token::Class::Key, "=="}, {"CMP_ARG"}}, NodeOutput::ReplaceInParent},
		{"CMP_NEQ", {{Token::Class::Key, "!="}, {"CMP_ARG"}}, NodeOutput::ReplaceInParent},
		{"CMP_LESS", {{Token::Class::Key, "<"}, {"CMP_ARG"}}, NodeOutput::ReplaceInParent},
		{"CMP_LEQ", {{Token::Class::Key, "<="}, {"CMP_ARG"}}, NodeOutput::ReplaceInParent},
		{"CMP_GT", {{Token::Class::Key, ">"}, {"CMP_ARG"}}, NodeOutput::ReplaceInParent},
		{"CMP_GTEQ", {{Token::Class::Key, ">="}, {"CMP_ARG"}}, NodeOutput::ReplaceInParent},

		{"ADDITION", {{"MULTIPLICATION"}, {"ADD_SUB", Repeat::ZeroMany}}, NodeOutput::Parent},
		{"ADD_SUB", {{"ADDS"}, {"SUBS"}}, {Combine::Alternative, NodeOutput::Parent}},
		{"ADDS", {{"ADD", Repeat::ZeroMany}}, NodeOutput::ReplaceInParent},
		{"ADD", {{Token::Class::Key, "+"}, {"MULTIPLICATION"}}, NodeOutput::Parent},
		{"SUBS", {{"SUB", Repeat::ZeroMany}}, NodeOutput::ReplaceInParent},
		{"SUB", {{Token::Class::Key, "-"}, {"MULTIPLICATION"}}, NodeOutput::Parent},

		{"MULTIPLICATION", {{"POWER"}, {"MUL_DIV_REM", Repeat::ZeroMany}}, NodeOutput::Parent},
		{"MUL_DIV_REM", {{"MULS"}, {"DIVS"}, {"REMS"}}, {Combine::Alternative, NodeOutput::Parent}},
		{"MULS", {{"MUL", Repeat::ZeroMany}}, NodeOutput::ReplaceInParent},
		{"MUL", {{Token::Class::Key, "*"}, {"POWER"}}, NodeOutput::Parent},
		{"DIVS", {{"DIV", Repeat::ZeroMany}}, NodeOutput::ReplaceInParent},
		{"DIV", {{Token::Class::Key, "/"}, {"POWER"}}, NodeOutput::Parent},
		{"REMS", {{"REM", Repeat::ZeroMany}}, NodeOutput::ReplaceInParent},
		{"REM", {{Token::Class::Key, "%"}, {"POWER"}}, NodeOutput::Parent},

		{"POWER", {{"SIGNED"}, {"POWS", Repeat::ZeroOne}}, NodeOutput::Parent},
		{"POWS", {{"POW", Repeat::ZeroMany}}, NodeOutput::ReplaceInParent},
		{"POW", {{Token::Class::Key, "^"}, {"SIGNED"}}, NodeOutput::Parent},

		{"SIGNED", {{"NEG"}, {"POS"}}, {Combine::Alternative, NodeOutput::Parent}},
		{"NEG", {{Token::Class::Key, "-"}, {"VALUE"}}},
		{"POS", {{Token::Class::Key, "+", Repeat::ZeroOne}, {"VALUE"}}, NodeOutput::Parent},

		{"VALUE", {{Token::Class::Literal}, {"VAR_CALL_INDEXED"}}, {Combine::Alternative, NodeOutput::Parent}},
		{"VALUE_BASE", {{"ADDR"}, {"SUBEXPR_OR_IDENT"}}, {Combine::Alternative, NodeOutput::Parent}},
		{"ADDR", {{Token::Class::Key, "&"}, {"VALUE_BASE"}}},
		{"SUBEXPR_OR_IDENT", {{"EXPR_IN_PAREN"}, {"UNQUALIFIED_NAME"}}, {Combine::Alternative, NodeOutput::Parent}},
		{"EXPR_IN_PAREN", {{Token::Class::Key, "("}, {"EXPRESSION"}, {Token::Class::Key, ")"}}, NodeOutput::Parent},
		{"VAR_CALL_INDEXED", {{"VALUE_BASE"}, {"VALUE_QUALIFIERS", Repeat::ZeroMany}}, NodeOutput::Parent},
		{"VALUE_QUALIFIERS", {{"UNQUALIFIED_NAME_TAIL"}, {"INDEX"}, {"CALL"}}, {Combine::Alternative, NodeOutput::Parent}},
		{"INDEX", {{Token::Class::Key, "["}, {"EXPRESSION_LIST"}, {Token::Class::Key, "]"}}, NodeOutput::ReplaceInParent},
		{"CALL", {{Token::Class::Key, "("}, {"EXPRESSION_LIST", Repeat::ZeroOne}, {Token::Class::Key, ")"}}, NodeOutput::ReplaceInParent},

		{"EXPRESSION_LIST", {{"EXPRESSION"}, {"COMMA_EXPRESSION", Repeat::ZeroMany}}, NodeOutput::Parent},
		{"COMMA_EXPRESSION", {{Token::Class::Key, ","}, {"EXPRESSION"}}, NodeOutput::Parent},
	};

	return s_langRules;
}

}