
#include <cctype>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>


// Token kinds – values < 256 are ASCII characters
//
//
struct Expr;     // expression
struct Stmt;     // statement
struct Decl;     // top‑level declaration
struct Type;     // type expression
struct Pattern;  // match pattern
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
    TK_PLUS_EQ,
    TK_MINUS_EQ,
    TK_MUL_EQ,
    TK_DIV_EQ,
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

    if(peek(lex) == '.' && std::isdigit(static_cast<unsigned char>(peek_next(lex))))
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
    if(first == '+' && peek(lex) == '=')
    {
        advance(lex);
        return TK_PLUS_EQ;
    }
    if(first == '-' && peek(lex) == '=')
    {
        advance(lex);
        return TK_MINUS_EQ;
    }
    if(first == '*' && peek(lex) == '=')
    {
        advance(lex);
        return TK_MUL_EQ;
    }
    if(first == '/' && peek(lex) == '=')
    {
        advance(lex);
        return TK_DIV_EQ;
    }
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
                // Single-character punctuation used directly by the parser.
                switch(c)
                {
                    case ';':
                    case ',':
                    case '(':
                    case ')':
                    case '{':
                    case '}':
                    case '[':
                    case ']':
                        advance(lex);
                        return static_cast<TokenKind>(c);
                    default:
                        // Unrecognised character
                        advance(lex);
                        return TK_ERROR;
                }
        }
    }
}

struct Token
{
    TokenKind        kind = TK_EOF;
    int              line = 1;
    int              col  = 1;
    std::string_view id;
    std::string      str_val;
    int64_t          int_val   = 0;
    double           float_val = 0.0;
};

Token lexer_take_token(Lexer* lex)
{
    Token t;
    t.kind      = token_next(lex);
    t.line      = lexer_line(lex);
    t.col       = lexer_col(lex);
    t.id        = lexer_id(lex);
    t.str_val   = std::string(lexer_str(lex));
    t.int_val   = lexer_int(lex);
    t.float_val = lexer_float(lex);
    return t;
}

enum NodeKind
{
    NODE_PROGRAM,
    NODE_BLOCK,
    NODE_DECL_VAR,
    NODE_DECL_FUNC,
    NODE_DECL_STRUCT,
    NODE_FIELD,
    NODE_PARAM,
    NODE_RETURN,
    NODE_FOR,
    NODE_EXPR_STMT,
    NODE_NAME_LIST,
    NODE_IDENT,
    NODE_INT,
    NODE_FLOAT,
    NODE_STRING,
    NODE_CHAR,
    NODE_BOOL,
    NODE_NULL,
    NODE_PREFIX,
    NODE_BINARY,
    NODE_CALL,
    NODE_MEMBER,
    NODE_INDEX,
    NODE_GROUP,
    NODE_ARRAY_LITERAL,
    NODE_STRUCT_LITERAL,
    NODE_TYPE_NAME,
    NODE_TYPE_POINTER,
    NODE_TYPE_ARRAY,
    NODE_TYPE_SLICE,
    NODE_TYPE_FUNC,
    NODE_ERROR,
};

struct AstNode
{
    NodeKind         kind = NODE_ERROR;
    std::string      text;
    int              line = 1;
    int              col  = 1;
    std::vector<int> children;
};

struct Parser
{
    Lexer                lex;
    Token                current;
    Token                next;
    bool                 ok = true;
    std::string          error;
    std::vector<AstNode> nodes;
};

static const char* node_kind_name(NodeKind kind)
{
    switch(kind)
    {
        case NODE_PROGRAM:
            return "PROGRAM";
        case NODE_BLOCK:
            return "BLOCK";
        case NODE_DECL_VAR:
            return "DECL_VAR";
        case NODE_DECL_FUNC:
            return "DECL_FUNC";
        case NODE_DECL_STRUCT:
            return "DECL_STRUCT";
        case NODE_FIELD:
            return "FIELD";
        case NODE_PARAM:
            return "PARAM";
        case NODE_RETURN:
            return "RETURN";
        case NODE_FOR:
            return "FOR";
        case NODE_EXPR_STMT:
            return "EXPR_STMT";
        case NODE_NAME_LIST:
            return "NAME_LIST";
        case NODE_IDENT:
            return "IDENT";
        case NODE_INT:
            return "INT";
        case NODE_FLOAT:
            return "FLOAT";
        case NODE_STRING:
            return "STRING";
        case NODE_CHAR:
            return "CHAR";
        case NODE_BOOL:
            return "BOOL";
        case NODE_NULL:
            return "NULL";
        case NODE_PREFIX:
            return "PREFIX";
        case NODE_BINARY:
            return "BINARY";
        case NODE_CALL:
            return "CALL";
        case NODE_MEMBER:
            return "MEMBER";
        case NODE_INDEX:
            return "INDEX";
        case NODE_GROUP:
            return "GROUP";
        case NODE_ARRAY_LITERAL:
            return "ARRAY_LITERAL";
        case NODE_STRUCT_LITERAL:
            return "STRUCT_LITERAL";
        case NODE_TYPE_NAME:
            return "TYPE_NAME";
        case NODE_TYPE_POINTER:
            return "TYPE_POINTER";
        case NODE_TYPE_ARRAY:
            return "TYPE_ARRAY";
        case NODE_TYPE_SLICE:
            return "TYPE_SLICE";
        case NODE_TYPE_FUNC:
            return "TYPE_FUNC";
        case NODE_ERROR:
            return "ERROR";
    }
    return "UNKNOWN";
}

static std::string token_kind_name(TokenKind kind)
{
    switch(kind)
    {
        case TK_IDENT:
            return "IDENT";
        case TK_INT_LIT:
            return "INT";
        case TK_FLOAT_LIT:
            return "FLOAT";
        case TK_STRING_LIT:
            return "STRING";
        case TK_CHAR_LIT:
            return "CHAR";
        case TK_EOF:
            return "EOF";
        case TK_ERROR:
            return "ERROR";
        case TK_ASSIGN:
            return ":=";
        case TK_CONST_ASSIGN:
            return "::";
        case TK_EQ:
            return "==";
        case TK_NE:
            return "!=";
        case TK_LE:
            return "<=";
        case TK_GE:
            return ">=";
        case TK_SHL:
            return "<<";
        case TK_SHR:
            return ">>";
        case TK_RANGE:
            return "..";
        case TK_RANGE_INCL:
            return "..=";
        case TK_ARROW:
            return "->";
        case TK_FAT_ARROW:
            return "=>";
        case TK_TRIPLE_DOT:
            return "...";
        case TK_DEREF:
            return ".*";
        case TK_OPT_UNWRAP:
            return ".?";
        case TK_PLUS_EQ:
            return "+=";
        case TK_MINUS_EQ:
            return "-=";
        case TK_MUL_EQ:
            return "*=";
        case TK_DIV_EQ:
            return "/=";
        default:
            break;
    }

    if(kind < 256)
    {
        char ch = static_cast<char>(kind);
        return std::string(1, ch);
    }

    return "TOKEN";
}

static bool token_is_identifier_like(TokenKind kind)
{
    return kind == TK_IDENT || kind == KW_VOID || kind == KW_NEVER || kind == KW_MAYBE || kind == KW_TYPE
           || kind == KW_TRUE || kind == KW_FALSE || kind == KW_NULL;
}

static bool token_is_assignment(TokenKind kind)
{
    return kind == '=' || kind == TK_ASSIGN || kind == TK_PLUS_EQ || kind == TK_MINUS_EQ || kind == TK_MUL_EQ || kind == TK_DIV_EQ;
}

static int binding_power(TokenKind kind);

static int prefix_binding_power()
{
    return 70;
}

static int parser_add_node(Parser* p, NodeKind kind, std::string text = std::string(), int line = 0, int col = 0)
{
    if(line == 0)
        line = p->current.line;
    if(col == 0)
        col = p->current.col;

    p->nodes.push_back(AstNode{kind, std::move(text), line, col, {}});
    return static_cast<int>(p->nodes.size() - 1);
}

static void parser_add_child(Parser* p, int parent, int child)
{
    if(parent >= 0 && child >= 0)
        p->nodes[parent].children.push_back(child);
}

static void parser_advance(Parser* p)
{
    p->current = p->next;
    p->next    = lexer_take_token(&p->lex);
}

static void parser_init(Parser* p, std::string_view source)
{
    lexer_init(&p->lex, source);
    p->current = lexer_take_token(&p->lex);
    p->next    = lexer_take_token(&p->lex);
    p->ok      = true;
    p->error.clear();
    p->nodes.clear();
}

static void parser_fail(Parser* p, const std::string& message)
{
    if(p->ok)
    {
        p->ok    = false;
        p->error = message + " at " + std::to_string(p->current.line) + ":" + std::to_string(p->current.col);
    }
}

static bool parser_match(Parser* p, int kind)
{
    if(p->current.kind == kind)
    {
        parser_advance(p);
        return true;
    }
    return false;
}

static bool parser_expect(Parser* p, int kind, const char* what)
{
    if(parser_match(p, kind))
        return true;
    parser_fail(p, std::string("expected ") + what + ", found " + token_kind_name(p->current.kind));
    return false;
}

static void parser_synchronize(Parser* p)
{
    while(p->current.kind != TK_EOF && p->current.kind != ';' && p->current.kind != '}' && p->current.kind != ')')
        parser_advance(p);
    if(p->current.kind == ';')
        parser_advance(p);
}

static int parse_expression(Parser* p, int rbp = 0);
static int parse_type(Parser* p);
static int parse_block(Parser* p);
static int parse_item(Parser* p);

static int parse_name_list(Parser* p)
{
    int list = parser_add_node(p, NODE_NAME_LIST);

    if(p->current.kind != TK_IDENT)
    {
        parser_fail(p, "expected identifier");
        parser_add_child(p, list, parser_add_node(p, NODE_ERROR, token_kind_name(p->current.kind)));
        return list;
    }

    while(true)
    {
        parser_add_child(p, list, parser_add_node(p, NODE_IDENT, std::string(p->current.id)));
        parser_advance(p);
        if(p->current.kind != ',')
            break;
        parser_advance(p);
        if(p->current.kind != TK_IDENT)
        {
            parser_fail(p, "expected identifier after comma");
            break;
        }
    }

    return list;
}

static int parse_type_name(Parser* p)
{
    if(token_is_identifier_like(p->current.kind) || p->current.kind == TK_IDENT)
    {
        std::string text;
        if(p->current.kind == TK_IDENT)
            text = std::string(p->current.id);
        else
            text = token_kind_name(p->current.kind);
        int node = parser_add_node(p, NODE_TYPE_NAME, text);
        parser_advance(p);
        return node;
    }

    parser_fail(p, "expected type name");
    return parser_add_node(p, NODE_ERROR, token_kind_name(p->current.kind));
}

static int parse_param_list(Parser* p)
{
    int list = parser_add_node(p, NODE_NAME_LIST);

    if(p->current.kind == ')')
        return list;

    while(true)
    {
        int param = parser_add_node(p, NODE_PARAM);
        int names = parse_name_list(p);
        parser_add_child(p, param, names);

        if(!parser_expect(p, ':', "':' in parameter"))
            break;

        int type = parse_type(p);
        parser_add_child(p, param, type);
        parser_add_child(p, list, param);

        if(p->current.kind != ',')
            break;
        parser_advance(p);
        if(p->current.kind == ')')
            break;
    }

    return list;
}

static int parse_type(Parser* p)
{
    if(p->current.kind == '*')
    {
        int line = p->current.line;
        int col  = p->current.col;
        parser_advance(p);

        bool has_mut = false;
        if(p->current.kind == TK_IDENT && p->current.id == "mut")
        {
            has_mut = true;
            parser_advance(p);
        }

        int node  = parser_add_node(p, NODE_TYPE_POINTER, has_mut ? "mut" : std::string(), line, col);
        int inner = parse_type(p);
        parser_add_child(p, node, inner);
        return node;
    }

    if(p->current.kind == '[')
    {
        int line = p->current.line;
        int col  = p->current.col;
        parser_advance(p);

        if(p->current.kind == ']')
        {
            parser_advance(p);
            int node = parser_add_node(p, NODE_TYPE_SLICE, std::string(), line, col);
            parser_add_child(p, node, parse_type(p));
            return node;
        }

        if(p->current.kind == TK_RANGE)
        {
            parser_advance(p);
            parser_expect(p, ']', "']' after dynamic array type");
            int node = parser_add_node(p, NODE_TYPE_ARRAY, "..", line, col);
            parser_add_child(p, node, parse_type(p));
            return node;
        }

        int size_expr = parse_expression(p, 0);
        parser_expect(p, ']', "']' after array size");
        int node = parser_add_node(p, NODE_TYPE_ARRAY, std::string(), line, col);
        parser_add_child(p, node, size_expr);
        parser_add_child(p, node, parse_type(p));
        return node;
    }

    if(p->current.kind == '(')
    {
        int line = p->current.line;
        int col  = p->current.col;
        parser_advance(p);
        int params = parse_param_list(p);
        parser_expect(p, ')', "')' after function type parameters");

        int node = parser_add_node(p, NODE_TYPE_FUNC, std::string(), line, col);
        parser_add_child(p, node, params);

        if(p->current.kind == TK_ARROW)
        {
            parser_advance(p);
            parser_add_child(p, node, parse_type(p));
        }
        else
        {
            parser_add_child(p, node, parser_add_node(p, NODE_TYPE_NAME, "void"));
        }
        return node;
    }

    return parse_type_name(p);
}

static int parse_expression_list(Parser* p, TokenKind end_kind)
{
    int list = parser_add_node(p, NODE_NAME_LIST);

    if(p->current.kind == end_kind)
        return list;

    while(p->current.kind != TK_EOF && p->current.kind != end_kind)
    {
        parser_add_child(p, list, parse_expression(p, 0));
        if(p->current.kind != ',')
            break;
        parser_advance(p);
    }

    return list;
}

static int parse_struct_literal(Parser* p, int dot_line, int dot_col)
{
    parser_expect(p, '{', "'{' after '.' for struct literal");
    int node = parser_add_node(p, NODE_STRUCT_LITERAL, std::string(), dot_line, dot_col);

    while(p->current.kind != TK_EOF && p->current.kind != '}')
    {
        if(p->current.kind == TK_IDENT && p->next.kind == '=')
        {
            int field = parser_add_node(p, NODE_FIELD, std::string(p->current.id));
            parser_advance(p);
            parser_advance(p);
            parser_add_child(p, field, parse_expression(p, 0));
            parser_add_child(p, node, field);
        }
        else
        {
            parser_add_child(p, node, parse_expression(p, 0));
        }

        if(p->current.kind == ',')
            parser_advance(p);
        else if(p->current.kind != '}')
            break;
    }

    parser_expect(p, '}', "'}' after struct literal");
    return node;
}

static int nud(Parser* p, const Token& token)
{
    switch(token.kind)
    {
        case TK_IDENT:
            return parser_add_node(p, NODE_IDENT, std::string(token.id), token.line, token.col);
        case TK_INT_LIT:
            return parser_add_node(p, NODE_INT, std::to_string(token.int_val), token.line, token.col);
        case TK_FLOAT_LIT:
            return parser_add_node(p, NODE_FLOAT, std::to_string(token.float_val), token.line, token.col);
        case TK_STRING_LIT:
            return parser_add_node(p, NODE_STRING, token.str_val, token.line, token.col);
        case TK_CHAR_LIT:
            return parser_add_node(p, NODE_CHAR, std::to_string(token.int_val), token.line, token.col);
        case KW_TRUE:
            return parser_add_node(p, NODE_BOOL, "true", token.line, token.col);
        case KW_FALSE:
            return parser_add_node(p, NODE_BOOL, "false", token.line, token.col);
        case KW_NULL:
            return parser_add_node(p, NODE_NULL, "null", token.line, token.col);
        case '(': {
            int group = parser_add_node(p, NODE_GROUP, std::string(), token.line, token.col);
            if(p->current.kind != ')')
                parser_add_child(p, group, parse_expression(p, 0));
            parser_expect(p, ')', "')' to close grouped expression");
            return group;
        }
        case '.':
            if(p->current.kind == '{')
                return parse_struct_literal(p, token.line, token.col);
            parser_fail(p, "expected '{' after '.' in literal");
            return parser_add_node(p, NODE_ERROR, ".", token.line, token.col);
        case '+':
        case '-':
        case '!':
        case '*': {
            std::string op(1, static_cast<char>(token.kind));
            if(token.kind == '*' && p->current.kind == TK_IDENT && p->current.id == "mut")
            {
                op = "*mut";
                parser_advance(p);
            }
            int node = parser_add_node(p, NODE_PREFIX, op, token.line, token.col);
            parser_add_child(p, node, parse_expression(p, prefix_binding_power()));
            return node;
        }
        default:
            parser_fail(p, std::string("unexpected token in expression: ") + token_kind_name(token.kind));
            return parser_add_node(p, NODE_ERROR, token_kind_name(token.kind), token.line, token.col);
    }
}

static int led(Parser* p, const Token& token, int left)
{
    switch(token.kind)
    {
        case '(': {
            int call = parser_add_node(p, NODE_CALL, std::string(), token.line, token.col);
            parser_add_child(p, call, left);
            if(p->current.kind != ')')
            {
                while(true)
                {
                    parser_add_child(p, call, parse_expression(p, 0));
                    if(p->current.kind != ',')
                        break;
                    parser_advance(p);
                }
            }
            parser_expect(p, ')', "')' after call arguments");
            return call;
        }
        case '[': {
            int index = parser_add_node(p, NODE_INDEX, std::string(), token.line, token.col);
            parser_add_child(p, index, left);
            if(p->current.kind != ']')
                parser_add_child(p, index, parse_expression(p, 0));
            parser_expect(p, ']', "']' after index expression");
            return index;
        }
        case '.': {
            if(p->current.kind != TK_IDENT)
            {
                parser_fail(p, "expected member name after '.'");
                return parser_add_node(p, NODE_ERROR, ".", token.line, token.col);
            }
            int member = parser_add_node(p, NODE_MEMBER, std::string(p->current.id), token.line, token.col);
            parser_add_child(p, member, left);
            parser_advance(p);
            return member;
        }
        case '=':
        case TK_ASSIGN:
        case TK_PLUS_EQ:
        case TK_MINUS_EQ:
        case TK_MUL_EQ:
        case TK_DIV_EQ: {
            int assign = parser_add_node(p, NODE_BINARY, token_kind_name(token.kind), token.line, token.col);
            parser_add_child(p, assign, left);
            parser_add_child(p, assign, parse_expression(p, binding_power(token.kind) - 1));
            return assign;
        }
        case TK_RANGE:
        case TK_RANGE_INCL:
        case TK_EQ:
        case TK_NE:
        case '<':
        case TK_LE:
        case '>':
        case TK_GE:
        case '+':
        case '-':
        case '*':
        case '/': {
            int binary = parser_add_node(p, NODE_BINARY, token_kind_name(token.kind), token.line, token.col);
            parser_add_child(p, binary, left);
            parser_add_child(p, binary, parse_expression(p, binding_power(token.kind)));
            return binary;
        }
        default:
            parser_fail(p, std::string("unexpected infix token: ") + token_kind_name(token.kind));
            return parser_add_node(p, NODE_ERROR, token_kind_name(token.kind), token.line, token.col);
    }
}

static int binding_power(TokenKind kind)
{
    switch(kind)
    {
        case '(':
        case '[':
        case '.':
            return 80;
        case '*':
        case '/':
            return 60;
        case '+':
        case '-':
            return 50;
        case TK_RANGE:
        case TK_RANGE_INCL:
            return 40;
        case TK_EQ:
        case TK_NE:
            return 35;
        case '<':
        case TK_LE:
        case '>':
        case TK_GE:
            return 30;
        case '=':
        case TK_ASSIGN:
        case TK_PLUS_EQ:
        case TK_MINUS_EQ:
        case TK_MUL_EQ:
        case TK_DIV_EQ:
            return 10;
        default:
            return 0;
    }
}

static int parse_expression(Parser* p, int rbp)
{
    Token token = p->current;
    parser_advance(p);
    int left = nud(p, token);

    while(rbp < binding_power(p->current.kind))
    {
        token = p->current;
        parser_advance(p);
        left = led(p, token, left);
    }

    return left;
}

static int parse_return_stmt(Parser* p)
{
    Token token = p->current;
    parser_advance(p);
    int node = parser_add_node(p, NODE_RETURN, std::string(), token.line, token.col);
    if(p->current.kind != ';' && p->current.kind != '}' && p->current.kind != TK_EOF)
        parser_add_child(p, node, parse_expression(p, 0));
    parser_match(p, ';');
    return node;
}

static int parse_for_stmt(Parser* p)
{
    Token token = p->current;
    parser_advance(p);
    int node = parser_add_node(p, NODE_FOR, std::string(), token.line, token.col);

    if(p->current.kind == TK_IDENT)
    {
        parser_add_child(p, node, parser_add_node(p, NODE_IDENT, std::string(p->current.id), p->current.line, p->current.col));
        parser_advance(p);
    }
    else
    {
        parser_fail(p, "expected loop variable after 'for'");
    }

    if(p->current.kind == ':' || p->current.kind == KW_IN)
        parser_advance(p);
    else
        parser_fail(p, "expected ':' or 'in' in for loop header");

    parser_add_child(p, node, parse_expression(p, 0));
    parser_add_child(p, node, parse_block(p));
    return node;
}

static int parse_field_like(Parser* p, NodeKind kind)
{
    int node  = parser_add_node(p, kind);
    int names = parse_name_list(p);
    parser_add_child(p, node, names);

    if(!parser_expect(p, ':', "':' after names"))
        return node;

    parser_add_child(p, node, parse_type(p));

    if(p->current.kind == '=')
    {
        parser_advance(p);
        parser_add_child(p, node, parse_expression(p, 0));
    }

    parser_match(p, ';');
    return node;
}

static int parse_decl(Parser* p)
{
    int line  = p->current.line;
    int col   = p->current.col;
    int names = parse_name_list(p);

    if(p->current.kind == TK_CONST_ASSIGN)
    {
        parser_advance(p);
        if(p->current.kind == KW_STRUCT)
        {
            parser_advance(p);
            int node = parser_add_node(p, NODE_DECL_STRUCT, std::string(), line, col);
            parser_add_child(p, node, names);
            parser_expect(p, '{', "'{' after struct declaration");
            while(p->current.kind != TK_EOF && p->current.kind != '}')
                parser_add_child(p, node, parse_field_like(p, NODE_FIELD));
            parser_expect(p, '}', "'}' after struct body");
            parser_match(p, ';');
            return node;
        }

        if(p->current.kind == '(')
        {
            parser_advance(p);
            int params = parse_param_list(p);
            parser_expect(p, ')', "')' after function parameters");

            int node = parser_add_node(p, NODE_DECL_FUNC, std::string(), line, col);
            parser_add_child(p, node, names);
            parser_add_child(p, node, params);

            if(p->current.kind == TK_ARROW)
            {
                parser_advance(p);
                parser_add_child(p, node, parse_type(p));
            }
            else
            {
                parser_add_child(p, node, parser_add_node(p, NODE_TYPE_NAME, "void"));
            }

            parser_add_child(p, node, parse_block(p));
            return node;
        }

        int node = parser_add_node(p, NODE_DECL_VAR, "const", line, col);
        parser_add_child(p, node, names);
        parser_add_child(p, node, parse_expression(p, 0));
        parser_match(p, ';');
        return node;
    }

    if(p->current.kind == TK_ASSIGN)
    {
        parser_advance(p);
        int node = parser_add_node(p, NODE_DECL_VAR, "mut", line, col);
        parser_add_child(p, node, names);
        parser_add_child(p, node, parse_expression(p, 0));
        parser_match(p, ';');
        return node;
    }

    if(p->current.kind == ':')
    {
        parser_advance(p);
        int node = parser_add_node(p, NODE_DECL_VAR, "typed", line, col);
        parser_add_child(p, node, names);
        parser_add_child(p, node, parse_type(p));

        if(p->current.kind == '=')
        {
            parser_advance(p);
            parser_add_child(p, node, parse_expression(p, 0));
        }

        parser_match(p, ';');
        return node;
    }

    parser_fail(p, "expected declaration operator after name");
    parser_synchronize(p);
    return parser_add_node(p, NODE_ERROR, "decl");
}

static int parse_statement(Parser* p)
{
    if(p->current.kind == KW_RETURN)
        return parse_return_stmt(p);
    if(p->current.kind == KW_FOR)
        return parse_for_stmt(p);
    if(p->current.kind == '{')
        return parse_block(p);

    if(p->current.kind == TK_IDENT
       && (p->next.kind == TK_CONST_ASSIGN || p->next.kind == TK_ASSIGN || p->next.kind == ':' || p->next.kind == ','))
        return parse_decl(p);

    int node = parser_add_node(p, NODE_EXPR_STMT, std::string(), p->current.line, p->current.col);
    parser_add_child(p, node, parse_expression(p, 0));
    parser_match(p, ';');
    return node;
}

static int parse_block(Parser* p)
{
    int line = p->current.line;
    int col  = p->current.col;
    parser_expect(p, '{', "'{' to start block");
    int node = parser_add_node(p, NODE_BLOCK, std::string(), line, col);

    while(p->current.kind != TK_EOF && p->current.kind != '}')
    {
        int item = parse_item(p);
        parser_add_child(p, node, item);
        if(!p->ok)
            parser_synchronize(p);
    }

    parser_expect(p, '}', "'}' to close block");
    return node;
}

static int parse_item(Parser* p)
{
    if(p->current.kind == TK_IDENT && p->next.kind == TK_CONST_ASSIGN)
        return parse_decl(p);
    if(p->current.kind == TK_IDENT && p->next.kind == TK_ASSIGN)
        return parse_decl(p);
    if(p->current.kind == TK_IDENT && (p->next.kind == ':' || p->next.kind == ','))
        return parse_decl(p);
    if(p->current.kind == KW_RETURN || p->current.kind == KW_FOR || p->current.kind == '{')
        return parse_statement(p);
    return parse_statement(p);
}

static int parse_program(Parser* p)
{
    int node = parser_add_node(p, NODE_PROGRAM, std::string(), 1, 1);
    while(p->current.kind != TK_EOF)
    {
        parser_add_child(p, node, parse_item(p));
        if(!p->ok)
            parser_synchronize(p);
    }
    return node;
}

static void print_ast(const Parser* p, int node, int depth = 0)
{
    const AstNode& n = p->nodes[node];
    for(int i = 0; i < depth; ++i)
        std::cout << "  ";
    std::cout << node_kind_name(n.kind);
    if(!n.text.empty())
        std::cout << " " << n.text;
    std::cout << " @" << n.line << ":" << n.col << '\n';
    for(int child : n.children)
        print_ast(p, child, depth + 1);
}


#include <fstream>
#include <sstream>

const char* load(const char* path)
{
    static std::string buffer;  // survives after function returns

    std::ifstream file(path, std::ios::binary);
    if(!file)
    {
        std::cerr << "Failed to open file: " << path << "\n";
        return nullptr;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    buffer = ss.str();

    return buffer.c_str();
}

// example_usage.cpp

int main()
{

    const char* code = load("examples/ok.mu");


    Lexer lex;
    lexer_init(&lex, code);

    Parser parser;
    parser_init(&parser, code);

    int root = parse_program(&parser);

    if(!parser.ok)
    {
        std::cerr << "parse error: " << parser.error << '\n';
        return 1;
    }

    print_ast(&parser, root);

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
