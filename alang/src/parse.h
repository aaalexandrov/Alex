#pragma once

#include "token.h"
#include <variant>

namespace alang {

enum class Repeat : uint32_t {
	One,
	ZeroOne,
	ZeroMany,
};
enum class Output : uint32_t {
	Disable,
	Enable,
	Auto,
};

struct MatchOptions {
	Repeat _repeat : 2;
	Output _output : 2;

	MatchOptions(Output out) : _repeat(Repeat::One), _output(out) {}
	MatchOptions(Repeat rep = Repeat::One, Output out = Output::Auto) : _repeat(rep), _output(out) {}
};

enum class Combine : uint32_t {
	Sequence,
	Alternative,
};

enum class Rename : uint32_t {
	Disable,
	Enable,
};

enum class NodeOutput : uint32_t {
	Own,
	Parent,
	ReplaceInParent,
};

struct ParseOptions {
	Combine _combine : 1;
	NodeOutput _nodeOutput : 2;
	Rename _rename : 1;

	ParseOptions(Combine combine = Combine::Sequence, NodeOutput out = NodeOutput::Own, Rename ren = Rename::Enable)
		: _combine(combine), _nodeOutput(out), _rename(ren) {}
	ParseOptions(NodeOutput out, Rename ren = Rename::Enable)
		: _combine(Combine::Sequence), _nodeOutput(out), _rename(ren) {}
	ParseOptions(Rename ren)
		: _combine(Combine::Sequence), _nodeOutput(NodeOutput::Own), _rename(ren) {}
	ParseOptions(Combine combine, Rename ren)
		: _combine(combine), _nodeOutput(NodeOutput::Own), _rename(ren) {}
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
		MatchOptions _opt;
		std::variant<Terminal, RuleRef> _content;

		Match(Token::Class cls, String str = "", MatchOptions opt = {});
		Match(String subruleId, MatchOptions opt = {});

		Terminal const *GetTerminal() const { return std::get_if<Terminal>(&_content); }
		RuleRef const *GetSubrule() const { return std::get_if<RuleRef>(&_content); }
	};

	String _id;
	ParseOptions _opt;
	std::vector<Match> _matches;

	ParseRule(String id, std::initializer_list<Match> matches, ParseOptions opt = {});
};

struct ParseNode {
	struct Content {
		std::variant<Token, std::unique_ptr<ParseNode>> _tokenOrNode;
		Token const *GetToken() const { return std::get_if<Token>(&_tokenOrNode); }
		ParseNode const *GetNode() const { return std::holds_alternative<std::unique_ptr<ParseNode>>(_tokenOrNode) ? std::get<std::unique_ptr<ParseNode>>(_tokenOrNode).get() : nullptr; }

		Content() = default;
		Content(Token const &token) : _tokenOrNode(token) {}
		Content(std::unique_ptr<ParseNode> &&node) : _tokenOrNode(std::move(node)) {}
		PosInFile GetFilePos() const { return GetToken() ? GetToken()->_filePos : GetNode()->_filePos; }
	};

	ParseRule const *_rule;
	String _label;
	std::vector<Content> _content;
	PosInFile _filePos;

	ParseNode(ParseRule const *rule, PosInFile filePos) : _rule(rule), _filePos(filePos) {}

	int32_t GetContentSize() const { return int32_t(_content.size()); }
	Content const *GetContent(int32_t i) const { return i < GetContentSize() ? &_content[i] : nullptr; }
	Token const *GetToken(int32_t i) const { return _content[i].GetToken(); }
	ParseNode const *GetSubnode(int32_t i) const { return _content[i].GetNode(); }
	PosInFile GetContentFilePos(int32_t i) const { return GetContent(i)->GetFilePos(); }
};

struct Parser {
	std::vector<ParseRule> const &_rules;

	Parser(std::vector<ParseRule> const &rules);

	std::unique_ptr<ParseNode> Parse(Tokenizer &tokens) const;

	void Dump(ParseNode const *node, int32_t indent = 0) const;

protected:
	bool MatchRule(Tokenizer &tokens, ParseRule const &rule, std::unique_ptr<ParseNode> &node) const;
};

struct ParseRulesHolder {
	std::vector<ParseRule> _rules;

	ParseRulesHolder(std::initializer_list<ParseRule> rules);
	std::vector<String> GetKeyStrings() const;
};

ParseRulesHolder const &AlangRules();

}