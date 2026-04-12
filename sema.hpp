#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "lexer.hpp"

struct Binding
{
    std::string type_name;
    bool        is_mutable = true;
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
    static bool is_literal_kind(mu::TokenKind kind);

    bool at(mu::TokenKind kind, size_t lookahead = 0) const;
    std::string token_text(size_t lookahead = 0) const;
    std::string token_text_at(size_t token_index) const;
    void emit(size_t token_index, std::string message);
    void consume_semicolons();
    bool is_expression_starter(size_t token_index) const;
    size_t skip_expression(size_t token_index) const;
    bool parse_type_name(size_t cursor, std::string& type_name, size_t& next_cursor) const;
    bool parse_statement();
    bool parse_inferred_declaration(bool is_mutable);
    bool parse_typed_declaration();
    bool parse_reassignment();
    bool parse_member_reassignment();

    std::string                           source_;
    mu::Lexer                             lexer_;
    std::vector<mu::Token>                tokens_;
    std::vector<Diagnostic>               diagnostics_;
    std::unordered_map<std::string, Binding> bindings_;
    size_t                                index_ = 0;
};
