#include "sema.hpp"

#include <utility>

MiniSemanticChecker::MiniSemanticChecker(std::string source)
    : source_(std::move(source))
    , lexer_(source_)
{
    for(;;)
    {
        mu::Token token = lexer_.next();
        tokens_.push_back(token);
        if(token.kind == mu::TokenKind::EOF_TOKEN)
        {
            break;
        }
    }
}

const std::vector<Diagnostic>& MiniSemanticChecker::run()
{
    while(!at(mu::TokenKind::EOF_TOKEN))
    {
        if(!parse_statement())
        {
            index_++;
        }
    }
    return diagnostics_;
}

bool MiniSemanticChecker::is_literal_kind(mu::TokenKind kind)
{
    return kind == mu::TokenKind::INTEGER_LITERAL || kind == mu::TokenKind::FLOAT_LITERAL
           || kind == mu::TokenKind::STRING_LITERAL || kind == mu::TokenKind::CHAR_LITERAL
           || kind == mu::TokenKind::HERE_STRING;
}

bool MiniSemanticChecker::at(mu::TokenKind kind, size_t lookahead) const
{
    return index_ + lookahead < tokens_.size() && tokens_[index_ + lookahead].kind == kind;
}

std::string MiniSemanticChecker::token_text(size_t lookahead) const
{
    return std::string(tokens_[index_ + lookahead].lexeme);
}

std::string MiniSemanticChecker::token_text_at(size_t token_index) const
{
    return std::string(tokens_[token_index].lexeme);
}

void MiniSemanticChecker::emit(size_t token_index, std::string message)
{
    diagnostics_.push_back(Diagnostic{tokens_[token_index].start, std::move(message)});
}

void MiniSemanticChecker::consume_semicolons()
{
    while(at(mu::TokenKind::SEMICOLON))
    {
        index_++;
    }
}

bool MiniSemanticChecker::is_expression_starter(size_t token_index) const
{
    if(token_index >= tokens_.size())
    {
        return false;
    }

    mu::TokenKind kind = tokens_[token_index].kind;
    return kind == mu::TokenKind::IDENTIFIER || kind == mu::TokenKind::L_BRACE || kind == mu::TokenKind::L_PAREN
           || kind == mu::TokenKind::DOT || kind == mu::TokenKind::STAR || kind == mu::TokenKind::MINUS
           || kind == mu::TokenKind::BANG || is_literal_kind(kind);
}

size_t MiniSemanticChecker::skip_expression(size_t token_index) const
{
    size_t cursor     = token_index;
    int    paren_depth = 0;
    int    brace_depth = 0;

    while(cursor < tokens_.size())
    {
        mu::TokenKind kind = tokens_[cursor].kind;

        if(kind == mu::TokenKind::L_PAREN)
        {
            paren_depth++;
        }
        else if(kind == mu::TokenKind::R_PAREN)
        {
            if(paren_depth == 0 && brace_depth == 0)
            {
                break;
            }
            if(paren_depth > 0)
            {
                paren_depth--;
            }
        }
        else if(kind == mu::TokenKind::L_BRACE)
        {
            brace_depth++;
        }
        else if(kind == mu::TokenKind::R_BRACE)
        {
            if(brace_depth == 0 && paren_depth == 0)
            {
                break;
            }
            if(brace_depth > 0)
            {
                brace_depth--;
            }
        }
        else if(paren_depth == 0 && brace_depth == 0
                && (kind == mu::TokenKind::SEMICOLON || kind == mu::TokenKind::COMMA))
        {
            break;
        }
        else if(kind == mu::TokenKind::EOF_TOKEN)
        {
            break;
        }

        cursor++;
    }

    return cursor;
}

bool MiniSemanticChecker::parse_type_name(size_t cursor, std::string& type_name, size_t& next_cursor) const
{
    if(cursor >= tokens_.size())
    {
        return false;
    }

    while(cursor < tokens_.size() && tokens_[cursor].kind == mu::TokenKind::STAR)
    {
        cursor++;
    }

    if(cursor < tokens_.size() && tokens_[cursor].kind == mu::TokenKind::IDENTIFIER && token_text_at(cursor) == "mut")
    {
        cursor++;
    }

    if(cursor >= tokens_.size() || tokens_[cursor].kind != mu::TokenKind::IDENTIFIER)
    {
        return false;
    }

    type_name   = token_text_at(cursor);
    next_cursor = cursor + 1;
    return true;
}

bool MiniSemanticChecker::parse_statement()
{
    if(!at(mu::TokenKind::IDENTIFIER))
    {
        return false;
    }

    if(at(mu::TokenKind::ASSIGN, 1))
    {
        return parse_inferred_declaration(true);
    }

    if(at(mu::TokenKind::CONST_ASSIGN, 1))
    {
        return parse_inferred_declaration(false);
    }

    if(at(mu::TokenKind::DOT, 1) && at(mu::TokenKind::IDENTIFIER, 2) && at(mu::TokenKind::EQUALS_ASSIGN, 3))
    {
        return parse_member_reassignment();
    }

    if(at(mu::TokenKind::COLON, 1))
    {
        return parse_typed_declaration();
    }

    if(at(mu::TokenKind::EQUALS_ASSIGN, 1))
    {
        if(index_ > 0
           && (tokens_[index_ - 1].kind == mu::TokenKind::L_BRACE || tokens_[index_ - 1].kind == mu::TokenKind::COMMA))
        {
            return false;
        }
        return parse_reassignment();
    }

    return false;
}

bool MiniSemanticChecker::parse_inferred_declaration(bool is_mutable)
{
    // Keep function-like and type-definition forms out of this tiny checker.
    if(at(mu::TokenKind::L_PAREN, 2) || at(mu::TokenKind::KW_STRUCT, 2) || at(mu::TokenKind::KW_ENUM, 2)
       || at(mu::TokenKind::KW_UNION, 2))
    {
        return false;
    }

    if(!is_expression_starter(index_ + 2))
    {
        return false;
    }

    std::string name = token_text();
    if(bindings_.count(name) != 0)
    {
        emit(index_, "redeclaration of '" + name + "'");
    }
    else
    {
        bindings_.insert({name, Binding{"auto", is_mutable}});
    }

    index_ = skip_expression(index_ + 2);
    consume_semicolons();
    return true;
}

bool MiniSemanticChecker::parse_typed_declaration()
{
    std::string name = token_text();

    std::string type_name;
    size_t      cursor = index_ + 2;
    if(!parse_type_name(cursor, type_name, cursor))
    {
        emit(index_ + 1, "expected explicit type name after ':'");
        index_ += 2;
        consume_semicolons();
        return true;
    }

    if(cursor < tokens_.size() && tokens_[cursor].kind == mu::TokenKind::EQUALS_ASSIGN)
    {
        size_t value_start = cursor + 1;
        if(!is_expression_starter(value_start))
        {
            emit(cursor, "expected expression after '='");
            cursor = value_start;
        }
        else
        {
            cursor = skip_expression(value_start);
        }
    }

    // Prototype rule: function parameter names are not tracked in global bindings.
    if(cursor < tokens_.size()
       && (tokens_[cursor].kind == mu::TokenKind::COMMA || tokens_[cursor].kind == mu::TokenKind::R_PAREN))
    {
        index_ = cursor;
        return true;
    }

    if(bindings_.count(name) != 0)
    {
        emit(index_, "redeclaration of '" + name + "'");
    }
    else
    {
        bindings_.insert({name, Binding{type_name, true}});
    }

    index_ = cursor;
    consume_semicolons();
    return true;
}

bool MiniSemanticChecker::parse_reassignment()
{
    std::string name = token_text();

    size_t value_start = index_ + 2;
    if(!is_expression_starter(value_start))
    {
        emit(index_ + 1, "expected expression after '='");
        index_ += 2;
        consume_semicolons();
        return true;
    }

    auto it = bindings_.find(name);
    if(it == bindings_.end())
    {
        emit(index_, "cannot assign to undeclared name '" + name + "'");
    }
    else if(!it->second.is_mutable)
    {
        emit(index_, "cannot assign to immutable name '" + name + "'");
    }

    index_ = skip_expression(value_start);
    consume_semicolons();
    return true;
}

bool MiniSemanticChecker::parse_member_reassignment()
{
    size_t value_start = index_ + 4;
    if(!is_expression_starter(value_start))
    {
        emit(index_ + 3, "expected expression after '='");
        index_ += 4;
        consume_semicolons();
        return true;
    }

    index_ = skip_expression(value_start);
    consume_semicolons();
    return true;
}
