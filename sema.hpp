#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "lexer.hpp"

struct Binding
{
    std::string typeName;
    bool        isMutable = true;
};

struct Diagnostic
{
    mu::SourceLocation where;
    std::string        message;
};

class MiniSemanticChecker
{
public:
    explicit MiniSemanticChecker(std::string source);

    const std::vector<Diagnostic>& run();

private:
    static bool isLiteralKind(mu::TokenKind kind);

    bool at(mu::TokenKind kind, size_t lookahead = 0) const;
    std::string tokenText(size_t lookahead = 0) const;
    std::string tokenTextAt(size_t tokenIndex) const;
    void emit(size_t tokenIndex, std::string message);
    void consumeSemicolons();
    bool isExpressionStarter(size_t tokenIndex) const;
    size_t skipExpression(size_t tokenIndex) const;
    bool parseTypeName(size_t cursor, std::string& typeName, size_t& nextCursor) const;
    bool parseStatement();
    bool parseInferredDeclaration(bool isMutable);
    bool parseTypedDeclaration();
    bool parseReassignment();
    bool parseMemberReassignment();

    std::string                           source_;
    mu::Lexer                             lexer_;
    std::vector<mu::Token>                tokens_;
    std::vector<Diagnostic>               diagnostics_;
    std::unordered_map<std::string, Binding> bindings_;
    size_t                                index_ = 0;
};
