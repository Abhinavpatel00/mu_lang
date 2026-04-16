
#include <cctype>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>


// Token kinds – values < 256 are ASCII characters
//
//
struct Expr;        // expression
struct Stmt;        // statement
struct Decl;        // top‑level declaration
struct Type;        // type expression
struct Pattern;     // match pattern
typedef struct
{
    int line_number;
    int line_offset;
} lex_location;


enum TokenKind : int16_t
{

//single char have ascii but multichar may be not so we use  this    fact  in lexer
	TK_IDENT = 256,
    TK_INT_LIT,
    TK_FLOAT_LIT,
    TK_STRING_LIT,
    TK_CHAR_LIT,
    TK_EOF,
    TK_ERROR,

    // Multi‑char operators
    TK_ASSIGN,        // :=
    TK_CONST_ASSIGN,  // ::
    TK_EQ,            // ==
    TK_NE,            // !=
    TK_LE,            // <=
    TK_GE,            // >=
    TK_SHL,           // <<
    TK_SHR,           // >>
    TK_RANGE,         // ..
    TK_RANGE_INCL,    // ..=
    TK_ARROW,         // ->
    TK_FAT_ARROW,     // =>
    TK_TRIPLE_DOT,    // ...
    TK_DEREF,
    TK_OPT_UNWRAP,
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
};

static const std::unordered_map<std::string_view, int> keywords = {
    {"struct", KW_STRUCT},     {"enum", KW_ENUM},   {"union", KW_UNION}, {"comp", KW_COMP},   {"using", KW_USING},
    {"distinct", KW_DISTINCT}, {"trait", KW_TRAIT}, {"impl", KW_IMPL},   {"defer", KW_DEFER}, {"as", KW_AS},
    {"match", KW_MATCH},       {"if", KW_IF},       {"else", KW_ELSE},   {"for", KW_FOR},     {"in", KW_IN},
    {"return", KW_RETURN},     {"true", KW_TRUE},   {"false", KW_FALSE}, {"null", KW_NULL},   {"void", KW_VOID},
    {"never", KW_NEVER},       {"maybe", KW_MAYBE}, {"type", KW_TYPE}};

enum Char_type : uint8_t
{
    CC_OTHER,
    CC_LETTER,
    CC_DIGIT,
    CC_WHITESPACE,
    CC_OPERATOR,
};

static uint8_t char_table[256];
// we may  not need stringview here string view is just like a immutable reference avoid copying but we can also just use char *  
// string view implementation is so complex 
struct Lexer
{
    std::string_view src;
    size_t           pos  = 0;
    int              line = 1;
    int              col  = 1;

    // Last token info
    int              tok_line = 1;
    int              tok_col  = 1;
    std::string_view id;       // for TK_IDENT
    std::string      str_val;  // for strings (owned copy)
    int64_t          int_val   = 0;
    double           float_val = 0.0;
};
void      lexer_init(Lexer* lex, std::string_view source);
TokenKind token_next(Lexer* lex);  // returns TokenKind

// Helpers to read current token value
inline std::string_view lexer_id(const Lexer* lex)
{
    return lex->id;
}
inline std::string_view lexer_str(const Lexer* lex)
{
    return lex->str_val;
}
inline int64_t lexer_int(const Lexer* lex)
{
    return lex->int_val;
}
inline double lexer_float(const Lexer* lex)
{
    return lex->float_val;
}
inline int lexer_line(const Lexer* lex)
{
    return lex->tok_line;
}
inline int lexer_col(const Lexer* lex)
{
    return lex->tok_col;
}


// mu_lexer.cpp


char peek(const Lexer* lex)
{
    return lex->pos < lex->src.size() ? lex->src[lex->pos] : 0;
}
char peek_next(const Lexer* lex)
{
    return lex->pos + 1 < lex->src.size() ? lex->src[lex->pos + 1] : 0;
}
void advance(Lexer* lex)
{
    if(peek(lex) == '\n')
    {
        lex->line++;
        lex->col = 1;
    }
    else
        lex->col++;
    lex->pos++;
}

void skip_whitespace(Lexer* lex)
{
    while(true)
    {
        char c = peek(lex);
        if(c == ' ' || c == '\t' || c == '\r' || c == '\n')
            advance(lex);
        else
            break;
    }
}

void skip_line_comment(Lexer* lex)
{
    while(peek(lex) && peek(lex) != '\n')
        advance(lex);
}

void skip_block_comment(Lexer* lex)
{
    int depth = 1;
    while(depth > 0 && lex->pos < lex->src.size())
    {
        if(peek(lex) == '/' && peek_next(lex) == '*')
        {
            advance(lex);
            advance(lex);
            depth++;
        }
        else if(peek(lex) == '*' && peek_next(lex) == '/')
        {
            advance(lex);
            advance(lex);
            depth--;
        }
        else
        {
            advance(lex);
        }
    }
}


TokenKind lex_identifier(Lexer* lex)
{
    size_t start = lex->pos;
    advance(lex);
    while(std::isalnum(static_cast<unsigned char>(peek(lex))) || peek(lex) == '_')
        advance(lex);

    lex->id = lex->src.substr(start, lex->pos - start);

    auto it = keywords.find(lex->id);
    return (it != keywords.end()) ? static_cast<TokenKind>(it->second) : TK_IDENT;
}


TokenKind lex_number(Lexer* lex)
{
    size_t start    = lex->pos;  // <-- remember start
    bool   is_float = false;

    while(std::isdigit(peek(lex)))
        advance(lex);

    if(peek(lex) == '.')
    {
        is_float = true;
        advance(lex);
        while(std::isdigit(peek(lex)))
            advance(lex);
    }

    if(peek(lex) == 'e' || peek(lex) == 'E')
    {
        is_float = true;
        advance(lex);
        if(peek(lex) == '+' || peek(lex) == '-')
            advance(lex);
        while(std::isdigit(peek(lex)))
            advance(lex);
    }

    // Extract the substring using the saved start offset
    std::string num_str(lex->src.substr(start, lex->pos - start));

    if(is_float)
    {
        lex->float_val = std::stod(num_str);
        return TK_FLOAT_LIT;
    }
    else
    {
        lex->int_val = std::stoll(num_str);
        return TK_INT_LIT;
    }
}

TokenKind lex_string(Lexer* lex, char quote)
{
    advance(lex);  // opening quote
    lex->str_val.clear();
    while(peek(lex) && peek(lex) != quote)
    {
        if(peek(lex) == '\\')
        {
            advance(lex);
            char esc = peek(lex);
            advance(lex);
            switch(esc)
            {
                case 'n':
                    lex->str_val += '\n';
                    break;
                case 't':
                    lex->str_val += '\t';
                    break;
                case '\\':
                    lex->str_val += '\\';
                    break;
                case '"':
                    lex->str_val += '"';
                    break;
                case '\'':
                    lex->str_val += '\'';
                    break;
                default:
                    lex->str_val += esc;
            }
        }
        else
        {
            lex->str_val += peek(lex);
            advance(lex);
        }
    }
    if(peek(lex) == quote)
        advance(lex);
    if(quote == '"')
    {
        return TK_STRING_LIT;
    }
    else
    {
        lex->int_val = lex->str_val.empty() ? 0 : static_cast<unsigned char>(lex->str_val[0]);
        return TK_CHAR_LIT;
    }
}

TokenKind lex_operator(Lexer* lex, char first)
{
    advance(lex);
    // Multi‑char operators
    if(first == ':' && peek(lex) == '=')
    {
        advance(lex);
        return TK_ASSIGN;
    }
    if(first == ':' && peek(lex) == ':')
    {
        advance(lex);
        return TK_CONST_ASSIGN;
    }
    if(first == '=' && peek(lex) == '=')
    {
        advance(lex);
        return TK_EQ;
    }
    if(first == '!' && peek(lex) == '=')
    {
        advance(lex);
        return TK_NE;
    }
    if(first == '<' && peek(lex) == '=')
    {
        advance(lex);
        return TK_LE;
    }
    if(first == '>' && peek(lex) == '=')
    {
        advance(lex);
        return TK_GE;
    }
    if(first == '<' && peek(lex) == '<')
    {
        advance(lex);
        return TK_SHL;
    }
    if(first == '>' && peek(lex) == '>')
    {
        advance(lex);
        return TK_SHR;
    }
    if(first == '.' && peek(lex) == '.')
    {
        advance(lex);
        if(peek(lex) == '=')
        {
            advance(lex);
            return TK_RANGE_INCL;
        }
        if(peek(lex) == '.')
        {
            advance(lex);
            return TK_TRIPLE_DOT;
        }
        return TK_RANGE;
    }
    if(first == '-' && peek(lex) == '>')
    {
        advance(lex);
        return TK_ARROW;
    }
    if(first == '=' && peek(lex) == '>')
    {
        advance(lex);
        return TK_FAT_ARROW;
    }
    if(first == '.' && peek(lex) == '*')
    {
        advance(lex);
        return TK_DEREF;
    }
    if(first == '.' && peek(lex) == '?')
    {
        advance(lex);
        return TK_OPT_UNWRAP;
    }


    // Single char: ASCII value
    return static_cast<TokenKind>(first);
}

void init_char_table()
{
    for(int i = 0; i < 256; i++)
        char_table[i] = CC_OTHER;

    for(char c = 'a'; c <= 'z'; c++)
        char_table[(uint8_t)c] = CC_LETTER;
    for(char c = 'A'; c <= 'Z'; c++)
        char_table[(uint8_t)c] = CC_LETTER;
    char_table[(uint8_t)'_'] = CC_LETTER;

    for(char c = '0'; c <= '9'; c++)
        char_table[(uint8_t)c] = CC_DIGIT;

    char_table[(uint8_t)' ']  = CC_WHITESPACE;
    char_table[(uint8_t)'\t'] = CC_WHITESPACE;
    char_table[(uint8_t)'\n'] = CC_WHITESPACE;
    char_table[(uint8_t)'\r'] = CC_WHITESPACE;

    const char* ops = "+-*/=<>!:.";
    for(const char* p = ops; *p; ++p)
        char_table[(uint8_t)*p] = CC_OPERATOR;
}
void lexer_init(Lexer* lex, std::string_view source)
{
    init_char_table();
    *lex     = Lexer{};
    lex->src = source;
}


TokenKind token_next(Lexer* lex)
{
    while(true)
    {
        skip_whitespace(lex);

        // Handle comments (these are not in the char table)
        if(peek(lex) == '/')
        {
            advance(lex);
            if(peek(lex) == '/')
            {
                skip_line_comment(lex);
                continue;
            }
            if(peek(lex) == '*')
            {
                advance(lex);
                skip_block_comment(lex);
                continue;
            }
            // It was just a single '/'
            return static_cast<TokenKind>('/');
        }

        if(lex->pos >= lex->src.size())
            return TK_EOF;

        lex->tok_line = lex->line;
        lex->tok_col  = lex->col;

        char c = peek(lex);
        switch(char_table[static_cast<uint8_t>(c)])
        {
            case CC_LETTER:
                return lex_identifier(lex);
            case CC_DIGIT:
                return lex_number(lex);
            case CC_OPERATOR:
                return lex_operator(lex, c);
            default:
                // Unrecognised character
                advance(lex);
                return TK_ERROR;
        }
    }
}


























// example_usage.cpp

int main()
{

const char* code = R"(
/*==============================
=         GLOBAL VARS          =
==============================*/

gravity :: 9.81;          // immutable inferred (f32)
count := 42;              // mutable inferred (i64)
temperature: f32 = 36.6;  // explicit type
integer_test: i32 = 36;  // explicit type

/*==============================
=           STRUCTS            =
==============================*/

Vec3 :: struct {
    x, y, z: f32;   // grouped fields
};

Player :: struct {
    id: i32;
    pos: Vec3;
    health: f32 = 100.0;   // default value
};

/*==============================
=        FUNCTIONS             =
==============================*/

length :: (v: Vec3) -> f32 {
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

move :: (p: *mut Player, delta: Vec3) -> void {
    p.pos.x += delta.x;
    p.pos.y += delta.y;
    p.pos.z += delta.z;
}
add_vec :: (a, b: Vec3) -> Vec3 {
    return .{ a.x + b.x, a.y + b.y, a.z + b.z };
}

move_2 :: (p: *mut Player, delta: Vec3) -> void {
    p.pos =add_vec(p.pos,delta);
}
/*==============================
=            MAIN              =
==============================*/

main :: () {
    v := .{ x = 1.0, y = 2.0, z = 3.0 };
    v2 := .{ 4.0, 5.0, 6.0 };
    p := .{
        id = 1,
        pos = v
    };
    p_ptr := *mut p;
    p_ptr.pos.x = 10.0;
    fixed: [3] i32 = .{1, 2, 3};
    dynamic: [..] i32;
    print_array :: (arr: [] i32) {
        for i : 0..arr.count-1 {
            print(arr[i]);
        }
    }
    print_array(fixed);
    len := length(v);
    print(len);
}
)";



	Lexer lex;
    lexer_init(&lex, code);

// token stream from lexer then 
// we have to pass it to parser   
//

    for(int tk = token_next(&lex); tk != TK_EOF; tk = token_next(&lex))
    {
        std::cout << "line " << lexer_line(&lex) << ":" << lexer_col(&lex) << "  ";
        if(tk < 256)
        {
            std::cout << '\'' << char(tk) << '\'';
        }
        else
        {
            switch(tk)
            {
                case TK_IDENT:
                    std::cout << "IDENT(" << lexer_id(&lex) << ")";
                    break;
                case TK_INT_LIT:
                    std::cout << "INT(" << lexer_int(&lex) << ")";
                    break;
                case TK_FLOAT_LIT:
                    std::cout << "FLOAT(" << lexer_float(&lex) << ")";
                    break;
                case TK_STRING_LIT:
                    std::cout << "STRING(\"" << lexer_str(&lex) << "\")";
                    break;
                case TK_CHAR_LIT:
                    std::cout << "CHAR('" << char(lexer_int(&lex)) << "')";
                    break;
                case TK_ASSIGN:
                    std::cout << "ASSIGN";
                    break;
                case TK_CONST_ASSIGN:
                    std::cout << "CONST_ASSIGN";
                    break;
                // … other cases as needed
                default:
                    std::cout << "TOKEN(" << tk << ")";
                    break;
            }
        }
        std::cout << '\n';
    }
}
