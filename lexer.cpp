#include "lexer.hpp"

#include <cctype>
#include <sstream>

namespace mu {
// -----------------------------------------------------------------------------
// Keyword table
// -----------------------------------------------------------------------------
const std::unordered_map<std::string_view, TokenKind> Lexer::s_keywords = {
    {"struct", TokenKind::KwStruct}, {"enum", TokenKind::KwEnum},   {"union", TokenKind::KwUnion},
    {"comp", TokenKind::KwComp},     {"using", TokenKind::KwUsing}, {"distinct", TokenKind::KwDistinct},
    {"trait", TokenKind::KwTrait},   {"impl", TokenKind::KwImpl},   {"defer", TokenKind::KwDefer},
    {"as", TokenKind::KwAs},         {"match", TokenKind::KwMatch}, {"if", TokenKind::KwIf},
    {"else", TokenKind::KwElse},     {"for", TokenKind::KwFor},     {"in", TokenKind::KwIn},
    {"return", TokenKind::KwReturn}, {"true", TokenKind::KwTrue},   {"false", TokenKind::KwFalse},
    {"null", TokenKind::KwNull},     {"void", TokenKind::KwVoid},   {"never", TokenKind::KwNever},
    {"maybe", TokenKind::KwMaybe},   {"type", TokenKind::KwType},
};

// -----------------------------------------------------------------------------
// Token utility methods
// -----------------------------------------------------------------------------
bool Token::isKeyword() const
{
    return kind >= TokenKind::KwStruct && kind <= TokenKind::KwType;
}

bool Token::isLiteral() const
{
    return kind == TokenKind::IntegerLiteral || kind == TokenKind::FloatLiteral || kind == TokenKind::StringLiteral
           || kind == TokenKind::CharLiteral || kind == TokenKind::HereString;
}

bool Token::isOperator() const
{
    return kind >= TokenKind::Assign && kind <= TokenKind::Hash;
}

std::string Token::toString() const
{
    auto kindName = [](TokenKind k) -> const char* {
        switch(k)
        {
            case TokenKind::Eof:
                return "Eof";
            case TokenKind::Identifier:
                return "Identifier";
            case TokenKind::IntegerLiteral:
                return "IntegerLiteral";
            case TokenKind::FloatLiteral:
                return "FloatLiteral";
            case TokenKind::StringLiteral:
                return "StringLiteral";
            case TokenKind::CharLiteral:
                return "CharLiteral";
            case TokenKind::HereString:
                return "HereString";
            case TokenKind::Assign:
                return "Assign";
            case TokenKind::ConstAssign:
                return "ConstAssign";
            case TokenKind::EqualsAssign:
                return "EqualsAssign";
            case TokenKind::Equal:
                return "Equal";
            case TokenKind::NotEqual:
                return "NotEqual";
            case TokenKind::Less:
                return "Less";
            case TokenKind::LessEqual:
                return "LessEqual";
            case TokenKind::Greater:
                return "Greater";
            case TokenKind::GreaterEqual:
                return "GreaterEqual";
            case TokenKind::Plus:
                return "Plus";
            case TokenKind::Minus:
                return "Minus";
            case TokenKind::Star:
                return "Star";
            case TokenKind::Slash:
                return "Slash";
            case TokenKind::Percent:
                return "Percent";
            case TokenKind::Bang:
                return "Bang";
            case TokenKind::Amp:
                return "Amp";
            case TokenKind::Pipe:
                return "Pipe";
            case TokenKind::Caret:
                return "Caret";
            case TokenKind::ShiftLeft:
                return "ShiftLeft";
            case TokenKind::ShiftRight:
                return "ShiftRight";
            case TokenKind::Dot:
                return "Dot";
            case TokenKind::Range:
                return "Range";
            case TokenKind::RangeInclusive:
                return "RangeInclusive";
            case TokenKind::Arrow:
                return "Arrow";
            case TokenKind::FatArrow:
                return "FatArrow";
            case TokenKind::Colon:
                return "Colon";
            case TokenKind::Semicolon:
                return "Semicolon";
            case TokenKind::Comma:
                return "Comma";
            case TokenKind::LParen:
                return "LParen";
            case TokenKind::RParen:
                return "RParen";
            case TokenKind::LBrace:
                return "LBrace";
            case TokenKind::RBrace:
                return "RBrace";
            case TokenKind::LBracket:
                return "LBracket";
            case TokenKind::RBracket:
                return "RBracket";
            case TokenKind::Dollar:
                return "Dollar";
            case TokenKind::Question:
                return "Question";
            case TokenKind::Hash:
                return "Hash";
            case TokenKind::KwStruct:
                return "KwStruct";
            case TokenKind::KwEnum:
                return "KwEnum";
            case TokenKind::KwUnion:
                return "KwUnion";
            case TokenKind::KwComp:
                return "KwComp";
            case TokenKind::KwUsing:
                return "KwUsing";
            case TokenKind::KwDistinct:
                return "KwDistinct";
            case TokenKind::KwTrait:
                return "KwTrait";
            case TokenKind::KwImpl:
                return "KwImpl";
            case TokenKind::KwDefer:
                return "KwDefer";
            case TokenKind::KwAs:
                return "KwAs";
            case TokenKind::KwMatch:
                return "KwMatch";
            case TokenKind::KwIf:
                return "KwIf";
            case TokenKind::KwElse:
                return "KwElse";
            case TokenKind::KwFor:
                return "KwFor";
            case TokenKind::KwIn:
                return "KwIn";
            case TokenKind::KwReturn:
                return "KwReturn";
            case TokenKind::KwTrue:
                return "KwTrue";
            case TokenKind::KwFalse:
                return "KwFalse";
            case TokenKind::KwNull:
                return "KwNull";
            case TokenKind::KwVoid:
                return "KwVoid";
            case TokenKind::KwNever:
                return "KwNever";
            case TokenKind::KwMaybe:
                return "KwMaybe";
            case TokenKind::KwType:
                return "KwType";
            case TokenKind::Illegal:
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
        return makeToken(TokenKind::Eof);
    }

    char c = advance();

    // Single-character tokens
    switch(c)
    {
        case '(':
            return makeToken(TokenKind::LParen);
        case ')':
            return makeToken(TokenKind::RParen);
        case '{':
            return makeToken(TokenKind::LBrace);
        case '}':
            return makeToken(TokenKind::RBrace);
        case '[':
            return makeToken(TokenKind::LBracket);
        case ']':
            return makeToken(TokenKind::RBracket);
        case ',':
            return makeToken(TokenKind::Comma);
        case ';':
            return makeToken(TokenKind::Semicolon);
        case '$':
            return makeToken(TokenKind::Dollar);
        case '?':
            return makeToken(TokenKind::Question);
        case '#':
            return makeToken(TokenKind::Hash);
        case '+':
            return makeToken(TokenKind::Plus);
        case '-':
            return makeToken(match('>') ? TokenKind::Arrow : TokenKind::Minus);
        case '*':
            return makeToken(TokenKind::Star);
        case '%':
            return makeToken(TokenKind::Percent);
        case '&':
            return makeToken(TokenKind::Amp);
        case '|':
            return makeToken(TokenKind::Pipe);
        case '^':
            return makeToken(TokenKind::Caret);
        case '!':
            return makeToken(match('=') ? TokenKind::NotEqual : TokenKind::Bang);
        case '=':
            return makeToken(match('=') ? TokenKind::Equal : TokenKind::EqualsAssign);
        case '<':
            return makeToken(match('<') ? TokenKind::ShiftLeft : match('=') ? TokenKind::LessEqual : TokenKind::Less);
        case '>':
            return makeToken(match('>') ? TokenKind::ShiftRight :
                             match('=') ? TokenKind::GreaterEqual :
                                          TokenKind::Greater);
        case ':':
            if(match(':'))
            {
                return makeToken(TokenKind::ConstAssign);
            }
            if(match('='))
            {
                return makeToken(TokenKind::Assign);
            }
            return makeToken(TokenKind::Colon);
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
            return makeToken(TokenKind::Slash);
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
    Token token = makeToken(TokenKind::Illegal);
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
            return makeToken(TokenKind::RangeInclusive);
        }
        return makeToken(TokenKind::Range);
    }
    return makeToken(TokenKind::Dot);
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
        return makeToken(TokenKind::IntegerLiteral);
    }

    // Binary literals (0b...)
    if(m_current - m_start == 1 && m_source[m_start] == '0' && (peekChar() == 'b' || peekChar() == 'B'))
    {
        advance();  // consume 'b'
        while(peekChar() == '0' || peekChar() == '1')
        {
            advance();
        }
        return makeToken(TokenKind::IntegerLiteral);
    }

    return makeToken(isFloat ? TokenKind::FloatLiteral : TokenKind::IntegerLiteral);
}

Token Lexer::lexIdentifier()
{
    while(std::isalnum(static_cast<unsigned char>(peekChar())) || peekChar() == '_')
    {
        advance();
    }

    std::string_view lexeme = m_source.substr(m_start, m_current - m_start);
    auto             it     = s_keywords.find(lexeme);
    TokenKind        kind   = (it != s_keywords.end()) ? it->second : TokenKind::Identifier;

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
    return makeToken(TokenKind::StringLiteral);
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
    return makeToken(TokenKind::CharLiteral);
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
                token.kind   = TokenKind::HereString;
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
