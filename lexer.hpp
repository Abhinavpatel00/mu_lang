// lexer.hpp
#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <cstdint>
#include <optional>

namespace mu {

enum class TokenKind : uint8_t {
    // End of file
    EOF_TOKEN,

    // Identifiers & literals
    IDENTIFIER,
    INTEGER_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    CHAR_LITERAL,
    HERE_STRING,

    // Operators & punctuation
    ASSIGN,          // :=
    CONST_ASSIGN,    // ::
    EQUALS_ASSIGN,   // =
    EQUAL,           // ==
    NOT_EQUAL,       // !=
    LESS,            // <
    LESS_EQUAL,      // <=
    GREATER,         // >
    GREATER_EQUAL,   // >=
    PLUS,            // +
    MINUS,           // -
    STAR,            // *
    SLASH,           // /
    PERCENT,         // %
    BANG,            // !
    AMP,             // &
    PIPE,            // |
    CARET,           // ^
    SHIFT_LEFT,      // <<
    SHIFT_RIGHT,     // >>
    DOT,             // .
    RANGE,           // ..
    RANGE_INCLUSIVE, // ..=
    ARROW,           // ->
    FAT_ARROW,       // =>
    COLON,           // :
    SEMICOLON,       // ;
    COMMA,           // ,
    L_PAREN,         // (
    R_PAREN,         // )
    L_BRACE,         // {
    R_BRACE,         // }
    L_BRACKET,       // [
    R_BRACKET,       // ]
    DOLLAR,          // $
    QUESTION,        // ?
    HASH,            // #

    // Keywords
    KW_STRUCT,
    KW_ENUM,
    KW_UNION,
    KW_COMP,
    KW_USING,
    KW_DISTINCT,
    KW_TRAIT,
    KW_IMPL,
    KW_DEFER,
    KW_AS,
    KW_MATCH,
    KW_IF,
    KW_ELSE,
    KW_FOR,
    KW_IN,
    KW_RETURN,
    KW_TRUE,
    KW_FALSE,
    KW_NULL,
    KW_VOID,
    KW_NEVER,
    KW_MAYBE,
    KW_TYPE,

    // Special
    ILLEGAL,
};

struct SourceLocation {
    uint32_t line = 1;
    uint32_t column = 1;
    uint32_t offset = 0;
};

struct Token {
    TokenKind kind;
    std::string_view lexeme;
    SourceLocation start;
    SourceLocation end;

    bool isKeyword() const;
    bool isLiteral() const;
    bool isOperator() const;
    std::string toString() const;
};

class Lexer {
public:
    explicit Lexer(std::string_view source);

    Token next();
    Token peek();

    // For parser error recovery
    void synchronize();

private:
    std::string_view m_source;
    size_t m_start = 0;
    size_t m_current = 0;
    uint32_t m_line = 1;
    uint32_t m_column = 1;

    // Here-string state
    std::optional<std::string_view> m_hereDelim;
    uint32_t m_hereStartLine = 0;

    // Cached peek token
    std::optional<Token> m_peek;

    // Core helpers
    bool isAtEnd() const;
    char advance();
    char peekChar() const;
    char peekNext() const;
    bool match(char expected);
    void skipWhitespace();

    // Token creation
    Token makeToken(TokenKind kind);
    Token makeError(std::string_view message);
    SourceLocation currentLocation() const;

    // Complex lexing
    Token lexNumber();
    Token lexIdentifier();
    Token lexString();
    Token lexChar();
    Token lexDot();
    Token consumeHereString(std::string_view delim);
    void consumeLineComment();
    void consumeBlockComment();

    // Keyword map
    static const std::unordered_map<std::string_view, TokenKind> s_keywords;
};

} // namespace mu
