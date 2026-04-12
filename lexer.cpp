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
bool Token::is_keyword() const
{
    return kind >= TokenKind::KW_STRUCT && kind <= TokenKind::KW_TYPE;
}

bool Token::is_literal() const
{
    return kind == TokenKind::INTEGER_LITERAL || kind == TokenKind::FLOAT_LITERAL || kind == TokenKind::STRING_LITERAL
           || kind == TokenKind::CHAR_LITERAL || kind == TokenKind::HERE_STRING;
}

bool Token::is_operator() const
{
    return kind >= TokenKind::ASSIGN && kind <= TokenKind::HASH;
}

std::string Token::to_string() const
{
    auto kind_name = [](TokenKind k) -> const char* {
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
    out << kind_name(kind) << "('" << lexeme << "') @ " << start.line << ':' << start.column;
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
    if(m_here_delim)
    {
        return consume_here_string(*m_here_delim);
    }

    skip_whitespace();
    m_start = m_current;

    if(is_at_end())
    {
        return make_token(TokenKind::EOF_TOKEN);
    }

    char c = advance();

    // Single-character tokens
    switch(c)
    {
        case '(':
            return make_token(TokenKind::L_PAREN);
        case ')':
            return make_token(TokenKind::R_PAREN);
        case '{':
            return make_token(TokenKind::L_BRACE);
        case '}':
            return make_token(TokenKind::R_BRACE);
        case '[':
            return make_token(TokenKind::L_BRACKET);
        case ']':
            return make_token(TokenKind::R_BRACKET);
        case ',':
            return make_token(TokenKind::COMMA);
        case ';':
            return make_token(TokenKind::SEMICOLON);
        case '$':
            return make_token(TokenKind::DOLLAR);
        case '?':
            return make_token(TokenKind::QUESTION);
        case '#':
            return make_token(TokenKind::HASH);
        case '+':
            return make_token(TokenKind::PLUS);
        case '-':
            return make_token(match('>') ? TokenKind::ARROW : TokenKind::MINUS);
        case '*':
            return make_token(TokenKind::STAR);
        case '%':
            return make_token(TokenKind::PERCENT);
        case '&':
            return make_token(TokenKind::AMP);
        case '|':
            return make_token(TokenKind::PIPE);
        case '^':
            return make_token(TokenKind::CARET);
        case '!':
            return make_token(match('=') ? TokenKind::NOT_EQUAL : TokenKind::BANG);
        case '=':
            return make_token(match('=') ? TokenKind::EQUAL : TokenKind::EQUALS_ASSIGN);
        case '<':
            return make_token(match('<') ? TokenKind::SHIFT_LEFT : match('=') ? TokenKind::LESS_EQUAL : TokenKind::LESS);
        case '>':
            return make_token(match('>') ? TokenKind::SHIFT_RIGHT :
                             match('=') ? TokenKind::GREATER_EQUAL :
                                          TokenKind::GREATER);
        case ':':
            if(match(':'))
            {
                return make_token(TokenKind::CONST_ASSIGN);
            }
            if(match('='))
            {
                return make_token(TokenKind::ASSIGN);
            }
            return make_token(TokenKind::COLON);
        case '.':
            return lex_dot();
        case '/': {
            if(match('/'))
            {
                consume_line_comment();
                return next();
            }
            else if(match('*'))
            {
                consume_block_comment();
                return next();
            }
            return make_token(TokenKind::SLASH);
        }
        case '\'':
            return lex_char();
        case '"':
            return lex_string();
        default:
            if(std::isdigit(static_cast<unsigned char>(c)))
            {
                return lex_number();
            }
            if(std::isalpha(static_cast<unsigned char>(c)) || c == '_')
            {
                return lex_identifier();
            }
            return make_error("Unexpected character");
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

    while(!is_at_end())
    {
        if(peek_char() == ';')
        {
            advance();
            return;
        }

        switch(peek_char())
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
bool Lexer::is_at_end() const
{
    return m_current >= m_source.size();
}

char Lexer::advance()
{
    m_column++;
    return m_source[m_current++];
}

char Lexer::peek_char() const
{
    if(is_at_end())
        return '\0';
    return m_source[m_current];
}

char Lexer::peek_next() const
{
    if(m_current + 1 >= m_source.size())
        return '\0';
    return m_source[m_current + 1];
}

bool Lexer::match(char expected)
{
    if(is_at_end() || m_source[m_current] != expected)
    {
        return false;
    }
    m_current++;
    m_column++;
    return true;
}

void Lexer::skip_whitespace()
{
    while(!is_at_end())
    {
        char c = peek_char();
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
Token Lexer::make_token(TokenKind kind)
{
    Token token;
    token.kind   = kind;
    token.lexeme = m_source.substr(m_start, m_current - m_start);
    token.start = SourceLocation{m_line, m_column - static_cast<uint32_t>(token.lexeme.size()), static_cast<uint32_t>(m_start)};
    token.end = current_location();
    return token;
}

Token Lexer::make_error(std::string_view message)
{
    (void)message;
    Token token = make_token(TokenKind::ILLEGAL);
    return token;
}

SourceLocation Lexer::current_location() const
{
    return SourceLocation{m_line, m_column, static_cast<uint32_t>(m_current)};
}

// -----------------------------------------------------------------------------
// Complex lexing
// -----------------------------------------------------------------------------
Token Lexer::lex_dot()
{
    if(match('.'))
    {
        if(match('='))
        {
            return make_token(TokenKind::RANGE_INCLUSIVE);
        }
        return make_token(TokenKind::RANGE);
    }
    return make_token(TokenKind::DOT);
}

Token Lexer::lex_number()
{
    bool is_float = false;

    while(std::isdigit(static_cast<unsigned char>(peek_char())))
    {
        advance();
    }

    if(peek_char() == '.' && std::isdigit(static_cast<unsigned char>(peek_next())))
    {
        is_float = true;
        advance();  // consume '.'
        while(std::isdigit(static_cast<unsigned char>(peek_char())))
        {
            advance();
        }
    }

    if(peek_char() == 'e' || peek_char() == 'E')
    {
        is_float = true;
        advance();
        if(peek_char() == '+' || peek_char() == '-')
        {
            advance();
        }
        while(std::isdigit(static_cast<unsigned char>(peek_char())))
        {
            advance();
        }
    }

    // Hex literals (0x...)
    if(m_current - m_start == 1 && m_source[m_start] == '0' && (peek_char() == 'x' || peek_char() == 'X'))
    {
        advance();  // consume 'x'
        while(std::isxdigit(static_cast<unsigned char>(peek_char())))
        {
            advance();
        }
        return make_token(TokenKind::INTEGER_LITERAL);
    }

    // Binary literals (0b...)
    if(m_current - m_start == 1 && m_source[m_start] == '0' && (peek_char() == 'b' || peek_char() == 'B'))
    {
        advance();  // consume 'b'
        while(peek_char() == '0' || peek_char() == '1')
        {
            advance();
        }
        return make_token(TokenKind::INTEGER_LITERAL);
    }

    return make_token(is_float ? TokenKind::FLOAT_LITERAL : TokenKind::INTEGER_LITERAL);
}

Token Lexer::lex_identifier()
{
    while(std::isalnum(static_cast<unsigned char>(peek_char())) || peek_char() == '_')
    {
        advance();
    }

    std::string_view lexeme = m_source.substr(m_start, m_current - m_start);
    auto             it     = s_keywords.find(lexeme);
    TokenKind        kind   = (it != s_keywords.end()) ? it->second : TokenKind::IDENTIFIER;

    // Handle contextual keywords like 'comp' - they can be used as identifiers
    // in some contexts; that's a parser-level distinction.
    return make_token(kind);
}

Token Lexer::lex_string()
{
    while(peek_char() != '"' && !is_at_end())
    {
        if(peek_char() == '\n')
        {
            m_line++;
            m_column = 1;
        }
        if(peek_char() == '\\')
        {
            advance();  // skip escape
        }
        advance();
    }

    if(is_at_end())
    {
        return make_error("Unterminated string literal");
    }

    advance();  // closing quote
    return make_token(TokenKind::STRING_LITERAL);
}

Token Lexer::lex_char()
{
    if(peek_char() == '\\')
    {
        advance();  // escape
    }
    advance();  // character

    if(peek_char() != '\'')
    {
        return make_error("Unterminated character literal");
    }

    advance();  // closing quote
    return make_token(TokenKind::CHAR_LITERAL);
}

void Lexer::consume_line_comment()
{
    while(peek_char() != '\n' && !is_at_end())
    {
        advance();
    }
}

void Lexer::consume_block_comment()
{
    int nesting = 1;
    while(nesting > 0 && !is_at_end())
    {
        if(peek_char() == '/' && peek_next() == '*')
        {
            advance();
            advance();
            nesting++;
        }
        else if(peek_char() == '*' && peek_next() == '/')
        {
            advance();
            advance();
            nesting--;
        }
        else if(peek_char() == '\n')
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

Token Lexer::consume_here_string(std::string_view delim)
{
    size_t content_start  = m_current;
    size_t content_length = 0;

    while(!is_at_end())
    {
        if(peek_char() == '\n')
        {
            size_t line_start = m_current + 1;
            advance();  // newline
            m_line++;
            m_column = 1;

            // Check if the next line starts with the delimiter
            if(m_source.size() - line_start >= delim.size() && m_source.substr(line_start, delim.size()) == delim)
            {
                // Found delimiter
                Token token;
                token.kind   = TokenKind::HERE_STRING;
                token.lexeme = m_source.substr(content_start, content_length);
                token.start  = SourceLocation{m_here_start_line, 1, static_cast<uint32_t>(content_start)};
                token.end    = current_location();

                // Consume delimiter line
                m_current = line_start + delim.size();
                m_column += delim.size();
                m_here_delim.reset();
                return token;
            }
        }
        else
        {
            advance();
        }
        content_length = m_current - content_start;
    }

    return make_error("Unterminated here-string");
}
}  // namespace mu
