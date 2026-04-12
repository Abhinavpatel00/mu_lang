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
    Eof,

    // Identifiers & literals
    Identifier,
    IntegerLiteral,
    FloatLiteral,
    StringLiteral,
    CharLiteral,
    HereString,

    // Operators & punctuation
    Assign,         // :=
    ConstAssign,    // ::
    EqualsAssign,   // =
    Equal,          // ==
    NotEqual,       // !=
    Less,           // <
    LessEqual,      // <=
    Greater,        // >
    GreaterEqual,   // >=
    Plus,           // +
    Minus,          // -
    Star,           // *
    Slash,          // /
    Percent,        // %
    Bang,           // !
    Amp,            // &
    Pipe,           // |
    Caret,          // ^
    ShiftLeft,      // <<
    ShiftRight,     // >>
    Dot,            // .
    Range,          // ..
    RangeInclusive, // ..=
    Arrow,          // ->
    FatArrow,       // =>
    Colon,          // :
    Semicolon,      // ;
    Comma,          // ,
    LParen,         // (
    RParen,         // )
    LBrace,         // {
    RBrace,         // }
    LBracket,       // [
    RBracket,       // ]
    Dollar,         // $
    Question,       // ?
    Hash,           // #

    // Keywords
    KwStruct,
    KwEnum,
    KwUnion,
    KwComp,
    KwUsing,
    KwDistinct,
    KwTrait,
    KwImpl,
    KwDefer,
    KwAs,
    KwMatch,
    KwIf,
    KwElse,
    KwFor,
    KwIn,
    KwReturn,
    KwTrue,
    KwFalse,
    KwNull,
    KwVoid,
    KwNever,
    KwMaybe,
    KwType,

    // Special
    Illegal,
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
