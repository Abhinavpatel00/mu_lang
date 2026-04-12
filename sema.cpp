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
        if(token.kind == mu::TokenKind::Eof)
        {
            break;
        }
    }
}

const std::vector<Diagnostic>& MiniSemanticChecker::run()
{
    while(!at(mu::TokenKind::Eof))
    {
        if(!parseStatement())
        {
            index_++;
        }
    }
    return diagnostics_;
}

bool MiniSemanticChecker::isLiteralKind(mu::TokenKind kind)
{
    return kind == mu::TokenKind::IntegerLiteral || kind == mu::TokenKind::FloatLiteral
           || kind == mu::TokenKind::StringLiteral || kind == mu::TokenKind::CharLiteral
           || kind == mu::TokenKind::HereString;
}

bool MiniSemanticChecker::at(mu::TokenKind kind, size_t lookahead) const
{
    return index_ + lookahead < tokens_.size() && tokens_[index_ + lookahead].kind == kind;
}

std::string MiniSemanticChecker::tokenText(size_t lookahead) const
{
    return std::string(tokens_[index_ + lookahead].lexeme);
}

std::string MiniSemanticChecker::tokenTextAt(size_t tokenIndex) const
{
    return std::string(tokens_[tokenIndex].lexeme);
}

void MiniSemanticChecker::emit(size_t tokenIndex, std::string message)
{
    diagnostics_.push_back(Diagnostic{tokens_[tokenIndex].start, std::move(message)});
}

void MiniSemanticChecker::consumeSemicolons()
{
    while(at(mu::TokenKind::Semicolon))
    {
        index_++;
    }
}

bool MiniSemanticChecker::isExpressionStarter(size_t tokenIndex) const
{
    if(tokenIndex >= tokens_.size())
    {
        return false;
    }

    mu::TokenKind kind = tokens_[tokenIndex].kind;
    return kind == mu::TokenKind::Identifier || kind == mu::TokenKind::LBrace || kind == mu::TokenKind::LParen
           || kind == mu::TokenKind::Dot || kind == mu::TokenKind::Star || kind == mu::TokenKind::Minus
           || kind == mu::TokenKind::Bang || isLiteralKind(kind);
}

size_t MiniSemanticChecker::skipExpression(size_t tokenIndex) const
{
    size_t cursor     = tokenIndex;
    int    parenDepth = 0;
    int    braceDepth = 0;

    while(cursor < tokens_.size())
    {
        mu::TokenKind kind = tokens_[cursor].kind;

        if(kind == mu::TokenKind::LParen)
        {
            parenDepth++;
        }
        else if(kind == mu::TokenKind::RParen)
        {
            if(parenDepth == 0 && braceDepth == 0)
            {
                break;
            }
            if(parenDepth > 0)
            {
                parenDepth--;
            }
        }
        else if(kind == mu::TokenKind::LBrace)
        {
            braceDepth++;
        }
        else if(kind == mu::TokenKind::RBrace)
        {
            if(braceDepth == 0 && parenDepth == 0)
            {
                break;
            }
            if(braceDepth > 0)
            {
                braceDepth--;
            }
        }
        else if(parenDepth == 0 && braceDepth == 0
                && (kind == mu::TokenKind::Semicolon || kind == mu::TokenKind::Comma))
        {
            break;
        }
        else if(kind == mu::TokenKind::Eof)
        {
            break;
        }

        cursor++;
    }

    return cursor;
}

bool MiniSemanticChecker::parseTypeName(size_t cursor, std::string& typeName, size_t& nextCursor) const
{
    if(cursor >= tokens_.size())
    {
        return false;
    }

    while(cursor < tokens_.size() && tokens_[cursor].kind == mu::TokenKind::Star)
    {
        cursor++;
    }

    if(cursor < tokens_.size() && tokens_[cursor].kind == mu::TokenKind::Identifier && tokenTextAt(cursor) == "mut")
    {
        cursor++;
    }

    if(cursor >= tokens_.size() || tokens_[cursor].kind != mu::TokenKind::Identifier)
    {
        return false;
    }

    typeName   = tokenTextAt(cursor);
    nextCursor = cursor + 1;
    return true;
}

bool MiniSemanticChecker::parseStatement()
{
    if(!at(mu::TokenKind::Identifier))
    {
        return false;
    }

    if(at(mu::TokenKind::Assign, 1))
    {
        return parseInferredDeclaration(true);
    }

    if(at(mu::TokenKind::ConstAssign, 1))
    {
        return parseInferredDeclaration(false);
    }

    if(at(mu::TokenKind::Dot, 1) && at(mu::TokenKind::Identifier, 2) && at(mu::TokenKind::EqualsAssign, 3))
    {
        return parseMemberReassignment();
    }

    if(at(mu::TokenKind::Colon, 1))
    {
        return parseTypedDeclaration();
    }

    if(at(mu::TokenKind::EqualsAssign, 1))
    {
        if(index_ > 0
           && (tokens_[index_ - 1].kind == mu::TokenKind::LBrace || tokens_[index_ - 1].kind == mu::TokenKind::Comma))
        {
            return false;
        }
        return parseReassignment();
    }

    return false;
}

bool MiniSemanticChecker::parseInferredDeclaration(bool isMutable)
{
    // Keep function-like and type-definition forms out of this tiny checker.
    if(at(mu::TokenKind::LParen, 2) || at(mu::TokenKind::KwStruct, 2) || at(mu::TokenKind::KwEnum, 2)
       || at(mu::TokenKind::KwUnion, 2))
    {
        return false;
    }

    if(!isExpressionStarter(index_ + 2))
    {
        return false;
    }

    std::string name = tokenText();
    if(bindings_.count(name) != 0)
    {
        emit(index_, "redeclaration of '" + name + "'");
    }
    else
    {
        bindings_.insert({name, Binding{"auto", isMutable}});
    }

    index_ = skipExpression(index_ + 2);
    consumeSemicolons();
    return true;
}

bool MiniSemanticChecker::parseTypedDeclaration()
{
    std::string name = tokenText();

    std::string typeName;
    size_t      cursor = index_ + 2;
    if(!parseTypeName(cursor, typeName, cursor))
    {
        emit(index_ + 1, "expected explicit type name after ':'");
        index_ += 2;
        consumeSemicolons();
        return true;
    }

    if(cursor < tokens_.size() && tokens_[cursor].kind == mu::TokenKind::EqualsAssign)
    {
        size_t valueStart = cursor + 1;
        if(!isExpressionStarter(valueStart))
        {
            emit(cursor, "expected expression after '='");
            cursor = valueStart;
        }
        else
        {
            cursor = skipExpression(valueStart);
        }
    }

    if(bindings_.count(name) != 0)
    {
        emit(index_, "redeclaration of '" + name + "'");
    }
    else
    {
        bindings_.insert({name, Binding{typeName, true}});
    }

    index_ = cursor;
    consumeSemicolons();
    return true;
}

bool MiniSemanticChecker::parseReassignment()
{
    std::string name = tokenText();

    size_t valueStart = index_ + 2;
    if(!isExpressionStarter(valueStart))
    {
        emit(index_ + 1, "expected expression after '='");
        index_ += 2;
        consumeSemicolons();
        return true;
    }

    auto it = bindings_.find(name);
    if(it == bindings_.end())
    {
        emit(index_, "cannot assign to undeclared name '" + name + "'");
    }
    else if(!it->second.isMutable)
    {
        emit(index_, "cannot assign to immutable name '" + name + "'");
    }

    index_ = skipExpression(valueStart);
    consumeSemicolons();
    return true;
}

bool MiniSemanticChecker::parseMemberReassignment()
{
    size_t valueStart = index_ + 4;
    if(!isExpressionStarter(valueStart))
    {
        emit(index_ + 3, "expected expression after '='");
        index_ += 4;
        consumeSemicolons();
        return true;
    }

    index_ = skipExpression(valueStart);
    consumeSemicolons();
    return true;
}
