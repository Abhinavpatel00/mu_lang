#include "lexer.hpp"

#include <cctype>
#include <sstream>

namespace mu {
// -----------------------------------------------------------------------------
// Keyword table
// -----------------------------------------------------------------------------
const std::unordered_map<std::string_view, TokenKind> Lexer::s_keywords = {
    {"struct", TokenKind::KW_STRUCT}, {"enum", TokenKind::KW_ENUM},   {"union", TokenKind::KW_UNION},
    {"comp", TokenKind::KW_COMP},     {"using", TokenKind::KW_USING}, {"distinct", TokenKind::KW_DISTINCT},
    {"trait", TokenKind::KW_TRAIT},   {"impl", TokenKind::KW_IMPL},   {"defer", TokenKind::KW_DEFER},
    {"as", TokenKind::KW_AS},         {"match", TokenKind::KW_MATCH}, {"if", TokenKind::KW_IF},
    {"else", TokenKind::KW_ELSE},     {"for", TokenKind::KW_FOR},     {"in", TokenKind::KW_IN},
    {"return", TokenKind::KW_RETURN}, {"true", TokenKind::KW_TRUE},   {"false", TokenKind::KW_FALSE},
    {"null", TokenKind::KW_NULL},     {"void", TokenKind::KW_VOID},   {"never", TokenKind::KW_NEVER},
    {"maybe", TokenKind::KW_MAYBE},   {"type", TokenKind::KW_TYPE},
};

// -----------------------------------------------------------------------------
// Token utility methods
// -----------------------------------------------------------------------------
bool Token::isKeyword() const
{
    return kind >= TokenKind::KW_STRUCT && kind <= TokenKind::KW_TYPE;
}

bool Token::isLiteral() const
{
    return kind == TokenKind::INTEGER_LITERAL || kind == TokenKind::FLOAT_LITERAL || kind == TokenKind::STRING_LITERAL
           || kind == TokenKind::CHAR_LITERAL || kind == TokenKind::HERE_STRING;
}

bool Token::isOperator() const
{
    return kind >= TokenKind::ASSIGN && kind <= TokenKind::HASH;
}

std::string Token::toString() const
{
    auto kindName = [](TokenKind k) -> const char* {
        switch(k)
        {
            case TokenKind::EOF_TOKEN:
                return "Eof";
            case TokenKind::IDENTIFIER:
                return "Identifier";
            case TokenKind::INTEGER_LITERAL:
                return "IntegerLiteral";
            case TokenKind::FLOAT_LITERAL:
                return "FloatLiteral";
            case TokenKind::STRING_LITERAL:
                return "StringLiteral";
            case TokenKind::CHAR_LITERAL:
                return "CharLiteral";
            case TokenKind::HERE_STRING:
                return "HereString";
            case TokenKind::ASSIGN:
                return "Assign";
            case TokenKind::CONST_ASSIGN:
                return "ConstAssign";
            case TokenKind::EQUALS_ASSIGN:
                return "EqualsAssign";
            case TokenKind::EQUAL:
                return "Equal";
            case TokenKind::NOT_EQUAL:
                return "NotEqual";
            case TokenKind::LESS:
                return "Less";
            case TokenKind::LESS_EQUAL:
                return "LessEqual";
            case TokenKind::GREATER:
                return "Greater";
            case TokenKind::GREATER_EQUAL:
                return "GreaterEqual";
            case TokenKind::PLUS:
                return "Plus";
            case TokenKind::MINUS:
                return "Minus";
            case TokenKind::STAR:
                return "Star";
            case TokenKind::SLASH:
                return "Slash";
            case TokenKind::PERCENT:
                return "Percent";
            case TokenKind::BANG:
                return "Bang";
            case TokenKind::AMP:
                return "Amp";
            case TokenKind::PIPE:
                return "Pipe";
            case TokenKind::CARET:
                return "Caret";
            case TokenKind::SHIFT_LEFT:
                return "ShiftLeft";
            case TokenKind::SHIFT_RIGHT:
                return "ShiftRight";
            case TokenKind::DOT:
                return "Dot";
            case TokenKind::RANGE:
                return "Range";
            case TokenKind::RANGE_INCLUSIVE:
                return "RangeInclusive";
            case TokenKind::ARROW:
                return "Arrow";
            case TokenKind::FAT_ARROW:
                return "FatArrow";
            case TokenKind::COLON:
                return "Colon";
            case TokenKind::SEMICOLON:
                return "Semicolon";
            case TokenKind::COMMA:
                return "Comma";
            case TokenKind::L_PAREN:
                return "LParen";
            case TokenKind::R_PAREN:
                return "RParen";
            case TokenKind::L_BRACE:
                return "LBrace";
            case TokenKind::R_BRACE:
                return "RBrace";
            case TokenKind::L_BRACKET:
                return "LBracket";
            case TokenKind::R_BRACKET:
                return "RBracket";
            case TokenKind::DOLLAR:
                return "Dollar";
            case TokenKind::QUESTION:
                return "Question";
            case TokenKind::HASH:
                return "Hash";
            case TokenKind::KW_STRUCT:
                return "KwStruct";
            case TokenKind::KW_ENUM:
                return "KwEnum";
            case TokenKind::KW_UNION:
                return "KwUnion";
            case TokenKind::KW_COMP:
                return "KwComp";
            case TokenKind::KW_USING:
                return "KwUsing";
            case TokenKind::KW_DISTINCT:
                return "KwDistinct";
            case TokenKind::KW_TRAIT:
                return "KwTrait";
            case TokenKind::KW_IMPL:
                return "KwImpl";
            case TokenKind::KW_DEFER:
                return "KwDefer";
            case TokenKind::KW_AS:
                return "KwAs";
            case TokenKind::KW_MATCH:
                return "KwMatch";
            case TokenKind::KW_IF:
                return "KwIf";
            case TokenKind::KW_ELSE:
                return "KwElse";
            case TokenKind::KW_FOR:
                return "KwFor";
            case TokenKind::KW_IN:
                return "KwIn";
            case TokenKind::KW_RETURN:
                return "KwReturn";
            case TokenKind::KW_TRUE:
                return "KwTrue";
            case TokenKind::KW_FALSE:
                return "KwFalse";
            case TokenKind::KW_NULL:
                return "KwNull";
            case TokenKind::KW_VOID:
                return "KwVoid";
            case TokenKind::KW_NEVER:
                return "KwNever";
            case TokenKind::KW_MAYBE:
                return "KwMaybe";
            case TokenKind::KW_TYPE:
                return "KwType";
            case TokenKind::ILLEGAL:
                return "Illegal";
        }
        return "Unknown";
    };

    std::ostringstream out;
    out << kindName(kind) << "('" << lexeme << "') @ " << start.line << ':' << start.column;
    return out.str();
}

// -----------------------------------------------------------------------------
// Lexer constructor
// -----------------------------------------------------------------------------
Lexer::Lexer(std::string_view source)
    : m_source(source)
{
}

// -----------------------------------------------------------------------------
// Public interface
// -----------------------------------------------------------------------------
Token Lexer::next()
{
    if(m_peek)
    {
        Token t = *m_peek;
        m_peek.reset();
        return t;
    }

    // Handle here-string mode
    if(m_hereDelim)
    {
        return consumeHereString(*m_hereDelim);
    }

    skipWhitespace();
    m_start = m_current;

    if(isAtEnd())
    {
        return makeToken(TokenKind::EOF_TOKEN);
    }

    char c = advance();

    // Single-character tokens
    switch(c)
    {
        case '(':
            return makeToken(TokenKind::L_PAREN);
        case ')':
            return makeToken(TokenKind::R_PAREN);
        case '{':
            return makeToken(TokenKind::L_BRACE);
        case '}':
            return makeToken(TokenKind::R_BRACE);
        case '[':
            return makeToken(TokenKind::L_BRACKET);
        case ']':
            return makeToken(TokenKind::R_BRACKET);
        case ',':
            return makeToken(TokenKind::COMMA);
        case ';':
            return makeToken(TokenKind::SEMICOLON);
        case '$':
            return makeToken(TokenKind::DOLLAR);
        case '?':
            return makeToken(TokenKind::QUESTION);
        case '#':
            return makeToken(TokenKind::HASH);
        case '+':
            return makeToken(TokenKind::PLUS);
        case '-':
            return makeToken(match('>') ? TokenKind::ARROW : TokenKind::MINUS);
        case '*':
            return makeToken(TokenKind::STAR);
        case '%':
            return makeToken(TokenKind::PERCENT);
        case '&':
            return makeToken(TokenKind::AMP);
        case '|':
            return makeToken(TokenKind::PIPE);
        case '^':
            return makeToken(TokenKind::CARET);
        case '!':
            return makeToken(match('=') ? TokenKind::NOT_EQUAL : TokenKind::BANG);
        case '=':
            return makeToken(match('=') ? TokenKind::EQUAL : TokenKind::EQUALS_ASSIGN);
        case '<':
            return makeToken(match('<') ? TokenKind::SHIFT_LEFT : match('=') ? TokenKind::LESS_EQUAL : TokenKind::LESS);
        case '>':
            return makeToken(match('>') ? TokenKind::SHIFT_RIGHT :
                             match('=') ? TokenKind::GREATER_EQUAL :
                                          TokenKind::GREATER);
        case ':':
            if(match(':'))
            {
                return makeToken(TokenKind::CONST_ASSIGN);
            }
            if(match('='))
            {
                return makeToken(TokenKind::ASSIGN);
            }
            return makeToken(TokenKind::COLON);
        case '.':
            return lexDot();
        case '/': {
            if(match('/'))
            {
                consumeLineComment();
                return next();
            }
            else if(match('*'))
            {
                consumeBlockComment();
                return next();
            }
            return makeToken(TokenKind::SLASH);
        }
        case '\'':
            return lexChar();
        case '"':
            return lexString();
        default:
            if(std::isdigit(static_cast<unsigned char>(c)))
            {
                return lexNumber();
            }
            if(std::isalpha(static_cast<unsigned char>(c)) || c == '_')
            {
                return lexIdentifier();
            }
            return makeError("Unexpected character");
    }
}

Token Lexer::peek()
{
    if(!m_peek)
    {
        m_peek = next();
    }
    return *m_peek;
}

void Lexer::synchronize()
{
    m_peek.reset();

    while(!isAtEnd())
    {
        if(peekChar() == ';')
        {
            advance();
            return;
        }

        switch(peekChar())
        {
            case '{':
            case '}':
            case '(':
            case ')':
            case '[':
            case ']':
            case ':':
            case ',':
                return;
            default:
                advance();
        }
    }
}

// -----------------------------------------------------------------------------
// Core helpers
// -----------------------------------------------------------------------------
bool Lexer::isAtEnd() const
{
    return m_current >= m_source.size();
}

char Lexer::advance()
{
    m_column++;
    return m_source[m_current++];
}

char Lexer::peekChar() const
{
    if(isAtEnd())
        return '\0';
    return m_source[m_current];
}

char Lexer::peekNext() const
{
    if(m_current + 1 >= m_source.size())
        return '\0';
    return m_source[m_current + 1];
}

bool Lexer::match(char expected)
{
    if(isAtEnd() || m_source[m_current] != expected)
    {
        return false;
    }
    m_current++;
    m_column++;
    return true;
}

void Lexer::skipWhitespace()
{
    while(!isAtEnd())
    {
        char c = peekChar();
        switch(c)
        {
            case ' ':
            case '\t':
                advance();
                break;
            case '\n':
                m_line++;
                m_column = 1;
                advance();
                break;
            default:
                return;
        }
    }
}

// -----------------------------------------------------------------------------
// Token creation
// -----------------------------------------------------------------------------
Token Lexer::makeToken(TokenKind kind)
{
    Token token;
    token.kind   = kind;
    token.lexeme = m_source.substr(m_start, m_current - m_start);
    token.start = SourceLocation{m_line, m_column - static_cast<uint32_t>(token.lexeme.size()), static_cast<uint32_t>(m_start)};
    token.end = currentLocation();
    return token;
}

Token Lexer::makeError(std::string_view message)
{
    (void)message;
    Token token = makeToken(TokenKind::ILLEGAL);
    return token;
}

SourceLocation Lexer::currentLocation() const
{
    return SourceLocation{m_line, m_column, static_cast<uint32_t>(m_current)};
}

// -----------------------------------------------------------------------------
// Complex lexing
// -----------------------------------------------------------------------------
Token Lexer::lexDot()
{
    if(match('.'))
    {
        if(match('='))
        {
            return makeToken(TokenKind::RANGE_INCLUSIVE);
        }
        return makeToken(TokenKind::RANGE);
    }
    return makeToken(TokenKind::DOT);
}

Token Lexer::lexNumber()
{
    bool isFloat = false;

    while(std::isdigit(static_cast<unsigned char>(peekChar())))
    {
        advance();
    }

    if(peekChar() == '.' && std::isdigit(static_cast<unsigned char>(peekNext())))
    {
        isFloat = true;
        advance();  // consume '.'
        while(std::isdigit(static_cast<unsigned char>(peekChar())))
        {
            advance();
        }
    }

    if(peekChar() == 'e' || peekChar() == 'E')
    {
        isFloat = true;
        advance();
        if(peekChar() == '+' || peekChar() == '-')
        {
            advance();
        }
        while(std::isdigit(static_cast<unsigned char>(peekChar())))
        {
            advance();
        }
    }

    // Hex literals (0x...)
    if(m_current - m_start == 1 && m_source[m_start] == '0' && (peekChar() == 'x' || peekChar() == 'X'))
    {
        advance();  // consume 'x'
        while(std::isxdigit(static_cast<unsigned char>(peekChar())))
        {
            advance();
        }
        return makeToken(TokenKind::INTEGER_LITERAL);
    }

    // Binary literals (0b...)
    if(m_current - m_start == 1 && m_source[m_start] == '0' && (peekChar() == 'b' || peekChar() == 'B'))
    {
        advance();  // consume 'b'
        while(peekChar() == '0' || peekChar() == '1')
        {
            advance();
        }
        return makeToken(TokenKind::INTEGER_LITERAL);
    }

    return makeToken(isFloat ? TokenKind::FLOAT_LITERAL : TokenKind::INTEGER_LITERAL);
}

Token Lexer::lexIdentifier()
{
    while(std::isalnum(static_cast<unsigned char>(peekChar())) || peekChar() == '_')
    {
        advance();
    }

    std::string_view lexeme = m_source.substr(m_start, m_current - m_start);
    auto             it     = s_keywords.find(lexeme);
    TokenKind        kind   = (it != s_keywords.end()) ? it->second : TokenKind::IDENTIFIER;

    // Handle contextual keywords like 'comp' - they can be used as identifiers
    // in some contexts; that's a parser-level distinction.
    return makeToken(kind);
}

Token Lexer::lexString()
{
    while(peekChar() != '"' && !isAtEnd())
    {
        if(peekChar() == '\n')
        {
            m_line++;
            m_column = 1;
        }
        if(peekChar() == '\\')
        {
            advance();  // skip escape
        }
        advance();
    }

    if(isAtEnd())
    {
        return makeError("Unterminated string literal");
    }

    advance();  // closing quote
    return makeToken(TokenKind::STRING_LITERAL);
}

Token Lexer::lexChar()
{
    if(peekChar() == '\\')
    {
        advance();  // escape
    }
    advance();  // character

    if(peekChar() != '\'')
    {
        return makeError("Unterminated character literal");
    }

    advance();  // closing quote
    return makeToken(TokenKind::CHAR_LITERAL);
}

void Lexer::consumeLineComment()
{
    while(peekChar() != '\n' && !isAtEnd())
    {
        advance();
    }
}

void Lexer::consumeBlockComment()
{
    int nesting = 1;
    while(nesting > 0 && !isAtEnd())
    {
        if(peekChar() == '/' && peekNext() == '*')
        {
            advance();
            advance();
            nesting++;
        }
        else if(peekChar() == '*' && peekNext() == '/')
        {
            advance();
            advance();
            nesting--;
        }
        else if(peekChar() == '\n')
        {
            m_line++;
            m_column = 1;
            advance();
        }
        else
        {
            advance();
        }
    }
}

Token Lexer::consumeHereString(std::string_view delim)
{
    size_t contentStart  = m_current;
    size_t contentLength = 0;

    while(!isAtEnd())
    {
        if(peekChar() == '\n')
        {
            size_t lineStart = m_current + 1;
            advance();  // newline
            m_line++;
            m_column = 1;

            // Check if the next line starts with the delimiter
            if(m_source.size() - lineStart >= delim.size() && m_source.substr(lineStart, delim.size()) == delim)
            {
                // Found delimiter
                Token token;
                token.kind   = TokenKind::HERE_STRING;
                token.lexeme = m_source.substr(contentStart, contentLength);
                token.start  = SourceLocation{m_hereStartLine, 1, static_cast<uint32_t>(contentStart)};
                token.end    = currentLocation();

                // Consume delimiter line
                m_current = lineStart + delim.size();
                m_column += delim.size();
                m_hereDelim.reset();
                return token;
            }
        }
        else
        {
            advance();
        }
        contentLength = m_current - contentStart;
    }

    return makeError("Unterminated here-string");
}
}  // namespace mu
