#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/TargetParser/Triple.h"

struct SourceLocation
{
    int line;
    int col;
};

enum TokenKind
{
    TK_EOF = 0,
    TK_IDENT,
    TK_INT_LIT,
    TK_STRING_LIT,
    TK_PRINT,
    TK_INT_KW,
    TK_COLON,
    TK_COLON_COLON,
    TK_COLON_EQ,
    TK_EQ,
    TK_SEMI,
    TK_COMMA,
    TK_LPAREN,
    TK_RPAREN,
    TK_LBRACE,
    TK_RBRACE
};

struct Token
{
    TokenKind      kind;
    std::string    text;
    int64_t        int_value;
    SourceLocation loc;
};

struct Lexer
{
    std::string input;
    size_t      pos;
    int         line;
    int         col;

    Lexer(std::string src)
        : input(std::move(src))
        , pos(0)
        , line(1)
        , col(1)
    {
    }

    char peek() const
    {
        if(pos >= input.size())
        {
            return '\0';
        }
        return input[pos];
    }

    char get()
    {
        char c = peek();
        if(c == '\0')
        {
            return c;
        }
        pos++;
        if(c == '\n')
        {
            line++;
            col = 1;
        }
        else
        {
            col++;
        }
        return c;
    }

    void skip_whitespace()
    {
        while(true)
        {
            char c = peek();
            if(c == '\0')
            {
                return;
            }
            if(c == '/' && pos + 1 < input.size() && input[pos + 1] == '/')
            {
                while(c != '\n' && c != '\0')
                {
                    c = get();
                }
                continue;
            }
            if(std::isspace(static_cast<unsigned char>(c)))
            {
                get();
                continue;
            }
            return;
        }
    }

    Token next_token()
    {
        skip_whitespace();

        Token t;
        t.text.clear();
        t.int_value = 0;
        t.loc       = {line, col};

        char c = peek();
        if(c == '\0')
        {
            t.kind = TK_EOF;
            return t;
        }

        if(std::isalpha(static_cast<unsigned char>(c)) || c == '_')
        {
            std::string ident;
            ident.push_back(get());
            while(std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')
            {
                ident.push_back(get());
            }
            t.text = ident;
            if(ident == "print")
            {
                t.kind = TK_PRINT;
            }
            else if(ident == "int")
            {
                t.kind = TK_INT_KW;
            }
            else
            {
                t.kind = TK_IDENT;
            }
            return t;
        }

        if(c == '"')
        {
            get();
            std::string lit;
            while(true)
            {
                char ch = get();
                if(ch == '\0' || ch == '\n')
                {
                    break;
                }
                if(ch == '"')
                {
                    break;
                }
                if(ch == '\\')
                {
                    char esc = get();
                    if(esc == '\0')
                    {
                        break;
                    }
                    lit.push_back('\\');
                    lit.push_back(esc);
                    continue;
                }
                lit.push_back(ch);
            }
            t.kind = TK_STRING_LIT;
            t.text = lit;
            return t;
        }

        if(std::isdigit(static_cast<unsigned char>(c)))
        {
            std::string num;
            while(std::isdigit(static_cast<unsigned char>(peek())))
            {
                num.push_back(get());
            }
            t.kind      = TK_INT_LIT;
            t.text      = num;
            t.int_value = std::stoll(num);
            return t;
        }

        if(c == ':' && pos + 1 < input.size())
        {
            char next = input[pos + 1];
            if(next == '=')
            {
                get();
                get();
                t.kind = TK_COLON_EQ;
                return t;
            }
            if(next == ':')
            {
                get();
                get();
                t.kind = TK_COLON_COLON;
                return t;
            }
        }

        get();
        switch(c)
        {
            case ':':
                t.kind = TK_COLON;
                break;
            case '=':
                t.kind = TK_EQ;
                break;
            case ';':
                t.kind = TK_SEMI;
                break;
            case ',':
                t.kind = TK_COMMA;
                break;
            case '(':
                t.kind = TK_LPAREN;
                break;
            case ')':
                t.kind = TK_RPAREN;
                break;
            case '{':
                t.kind = TK_LBRACE;
                break;
            case '}':
                t.kind = TK_RBRACE;
                break;
            default:
                t.kind = TK_EOF;
                break;
        }
        return t;
    }
};

struct CodegenContext;

struct Expr
{
    SourceLocation loc;
    virtual llvm::Value* codegen(CodegenContext& ctx) = 0;
    virtual ~Expr() = default;
};

struct IntExpr : Expr
{
    int64_t value;

    IntExpr(int64_t v, SourceLocation l)
        : value(v)
    {
        loc = l;
    }

    llvm::Value* codegen(CodegenContext& ctx) override;
};

struct NameExpr : Expr
{
    std::string name;

    NameExpr(std::string n, SourceLocation l)
        : name(std::move(n))
    {
        loc = l;
    }

    llvm::Value* codegen(CodegenContext& ctx) override;
};

struct Stmt
{
    SourceLocation loc;
    virtual bool codegen(CodegenContext& ctx) = 0;
    virtual ~Stmt() = default;
};

struct VarDecl : Stmt
{
    std::string              name;
    std::unique_ptr<Expr>    init;

    VarDecl(std::string n, std::unique_ptr<Expr> i, SourceLocation l)
        : name(std::move(n))
        , init(std::move(i))
    {
        loc = l;
    }

    bool codegen(CodegenContext& ctx) override;
};

struct PrintStmt : Stmt
{
    std::unique_ptr<Expr> expr;

    PrintStmt(std::unique_ptr<Expr> e, SourceLocation l)
        : expr(std::move(e))
    {
        loc = l;
    }

    bool codegen(CodegenContext& ctx) override;
};

struct Parser
{
    Lexer lex;
    Token current;
    bool  ok;

    Parser(std::string src)
        : lex(std::move(src))
        , ok(true)
    {
        advance();
    }

    void advance()
    {
        current = lex.next_token();
    }

    void error(const std::string& msg)
    {
        if(!ok)
        {
            return;
        }
        ok = false;
        std::cerr << current.loc.line << ":" << current.loc.col << ": " << msg << "\n";
    }

    bool expect(TokenKind kind, const std::string& msg)
    {
        if(current.kind != kind)
        {
            error(msg);
            return false;
        }
        advance();
        return true;
    }

    std::unique_ptr<Expr> parse_expr()
    {
        if(current.kind == TK_INT_LIT)
        {
            auto expr = std::make_unique<IntExpr>(current.int_value, current.loc);
            advance();
            return expr;
        }
        if(current.kind == TK_IDENT)
        {
            auto expr = std::make_unique<NameExpr>(current.text, current.loc);
            advance();
            return expr;
        }
        error("expected expression");
        return nullptr;
    }

    std::unique_ptr<Stmt> parse_var_decl()
    {
        SourceLocation loc = current.loc;
        std::string name = current.text;
        advance();

        if(current.kind == TK_COLON_EQ)
        {
            advance();
            auto init = parse_expr();
            if(!init)
            {
                return nullptr;
            }
            if(!expect(TK_SEMI, "expected ';'"))
            {
                return nullptr;
            }
            return std::make_unique<VarDecl>(name, std::move(init), loc);
        }

        if(current.kind == TK_COLON)
        {
            advance();
            if(!expect(TK_INT_KW, "expected 'int'"))
            {
                return nullptr;
            }
            if(current.kind == TK_EQ)
            {
                advance();
                auto init = parse_expr();
                if(!init)
                {
                    return nullptr;
                }
                if(!expect(TK_SEMI, "expected ';'"))
                {
                    return nullptr;
                }
                return std::make_unique<VarDecl>(name, std::move(init), loc);
            }
            if(current.kind == TK_SEMI)
            {
                advance();
                auto init = std::make_unique<IntExpr>(0, loc);
                return std::make_unique<VarDecl>(name, std::move(init), loc);
            }
            error("expected '=' or ';'");
            return nullptr;
        }

        error("expected ':' or ':='");
        return nullptr;
    }

    std::unique_ptr<Stmt> parse_print()
    {
        SourceLocation loc = current.loc;
        advance();

        if(!expect(TK_LPAREN, "expected '('") )
        {
            return nullptr;
        }

        if(current.kind == TK_STRING_LIT)
        {
            advance();
            if(!expect(TK_COMMA, "expected ',' after format string"))
            {
                return nullptr;
            }
        }
        auto expr = parse_expr();
        if(!expr)
        {
            return nullptr;
        }
        if(current.kind == TK_COMMA)
        {
            advance();
            if(current.kind != TK_STRING_LIT)
            {
                error("expected string literal after ','");
                return nullptr;
            }
            advance();
        }
        if(!expect(TK_RPAREN, "expected ')'") )
        {
            return nullptr;
        }
        if(!expect(TK_SEMI, "expected ';'"))
        {
            return nullptr;
        }

        return std::make_unique<PrintStmt>(std::move(expr), loc);
    }

    std::vector<std::unique_ptr<Stmt>> parse_block()
    {
        std::vector<std::unique_ptr<Stmt>> stmts;

        while(current.kind != TK_RBRACE && current.kind != TK_EOF && ok)
        {
            if(current.kind == TK_IDENT)
            {
                auto stmt = parse_var_decl();
                if(stmt)
                {
                    stmts.push_back(std::move(stmt));
                }
                continue;
            }
            if(current.kind == TK_PRINT)
            {
                auto stmt = parse_print();
                if(stmt)
                {
                    stmts.push_back(std::move(stmt));
                }
                continue;
            }
            error("unexpected token in block");
            break;
        }

        if(!expect(TK_RBRACE, "expected '}'"))
        {
            return {};
        }

        return stmts;
    }

    std::vector<std::unique_ptr<Stmt>> parse_main_block()
    {
        advance();
        if(!expect(TK_COLON_COLON, "expected '::'"))
        {
            return {};
        }
        if(!expect(TK_LPAREN, "expected '('") )
        {
            return {};
        }
        if(!expect(TK_RPAREN, "expected ')'"))
        {
            return {};
        }
        if(!expect(TK_LBRACE, "expected '{'"))
        {
            return {};
        }
        return parse_block();
    }

    std::vector<std::unique_ptr<Stmt>> parse_program()
    {
        std::vector<std::unique_ptr<Stmt>> stmts;

        bool saw_main = false;

        while(current.kind != TK_EOF && ok)
        {
            if(!saw_main && current.kind == TK_IDENT && current.text == "main")
            {
                stmts = parse_main_block();
                saw_main = true;
                continue;
            }
            if(saw_main)
            {
                error("unexpected token after main block");
                break;
            }
            if(current.kind == TK_IDENT)
            {
                auto stmt = parse_var_decl();
                if(stmt)
                {
                    stmts.push_back(std::move(stmt));
                }
                continue;
            }
            if(current.kind == TK_PRINT)
            {
                auto stmt = parse_print();
                if(stmt)
                {
                    stmts.push_back(std::move(stmt));
                }
                continue;
            }
            error("unexpected token");
        }

        return stmts;
    }
};

struct CodegenContext
{
    llvm::LLVMContext               ctx;
    llvm::IRBuilder<>               builder;
    std::unique_ptr<llvm::Module>   module;
    llvm::Function*                 main_fn;
    llvm::Function*                 printf_fn;
    llvm::Value*                    fmt_string;
    std::map<std::string, llvm::AllocaInst*> locals;
    bool                            ok;

    CodegenContext()
        : builder(ctx)
        , main_fn(nullptr)
        , printf_fn(nullptr)
        , fmt_string(nullptr)
        , ok(true)
    {
    }

    void error_at(const SourceLocation& loc, const std::string& msg)
    {
        if(!ok)
        {
            return;
        }
        ok = false;
        std::cerr << loc.line << ":" << loc.col << ": " << msg << "\n";
    }
};

llvm::Value* IntExpr::codegen(CodegenContext& ctx)
{
    return llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx.ctx), value, true);
}

llvm::Value* NameExpr::codegen(CodegenContext& ctx)
{
    auto it = ctx.locals.find(name);
    if(it == ctx.locals.end())
    {
        ctx.error_at(loc, "unknown variable '" + name + "'");
        return nullptr;
    }
    return ctx.builder.CreateLoad(llvm::Type::getInt64Ty(ctx.ctx), it->second, name + "_load");
}

static llvm::AllocaInst* create_entry_alloca(llvm::Function* fn, llvm::LLVMContext& ctx, const std::string& name)
{
    llvm::IRBuilder<> tmp(&fn->getEntryBlock(), fn->getEntryBlock().begin());
    return tmp.CreateAlloca(llvm::Type::getInt64Ty(ctx), nullptr, name);
}

bool VarDecl::codegen(CodegenContext& ctx)
{
    if(ctx.locals.find(name) != ctx.locals.end())
    {
        ctx.error_at(loc, "redefinition of '" + name + "'");
        return false;
    }
    llvm::Value* init_val = init->codegen(ctx);
    if(!init_val)
    {
        return false;
    }
    llvm::AllocaInst* slot = create_entry_alloca(ctx.main_fn, ctx.ctx, name);
    ctx.builder.CreateStore(init_val, slot);
    ctx.locals[name] = slot;
    return true;
}

bool PrintStmt::codegen(CodegenContext& ctx)
{
    llvm::Value* value = expr->codegen(ctx);
    if(!value)
    {
        return false;
    }
    ctx.builder.CreateCall(ctx.printf_fn, {ctx.fmt_string, value});
    return true;
}

struct FileOutput
{
    std::string object_path;
    std::string exe_path;
};

static bool write_object_file(llvm::Module* module, const std::string& path)
{
    std::string  target_triple = llvm::sys::getDefaultTargetTriple();
    llvm::Triple triple(target_triple);
    module->setTargetTriple(triple);

    std::string         err;
    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(target_triple, err);
    if(!target)
    {
        llvm::errs() << err << "\n";
        return false;
    }

    llvm::TargetOptions  opt;
    llvm::TargetMachine* target_machine = target->createTargetMachine(triple, "generic", "", opt, llvm::Reloc::PIC_);
    module->setDataLayout(target_machine->createDataLayout());

    std::error_code      ec;
    llvm::raw_fd_ostream dest(path, ec, llvm::sys::fs::OF_None);
    if(ec)
    {
        llvm::errs() << "Could not open file: " << ec.message() << "\n";
        return false;
    }

    llvm::legacy::PassManager pass;
    if(target_machine->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile))
    {
        llvm::errs() << "Target machine cannot emit object file\n";
        return false;
    }

    pass.run(*module);
    dest.flush();
    delete target_machine;
    return true;
}

static bool link_executable(const std::string& object_path, const std::string& exe_path)
{
    std::ostringstream cmd;
    cmd << "cc " << object_path << " -o " << exe_path;
    int rc = std::system(cmd.str().c_str());
    return rc == 0;
}

static std::string read_file_text(const std::string& path)
{
    std::ifstream      in(path);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        std::cerr << "usage: mucc <file.mu> [-o output]\n";
        return 1;
    }

    std::string input_path;
    std::string output_path = "a.out";

    for(int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if(arg == "-o" && i + 1 < argc)
        {
            output_path = argv[++i];
            continue;
        }
        input_path = arg;
    }

    if(input_path.empty())
    {
        std::cerr << "error: no input file provided\n";
        return 1;
    }

    std::string src = read_file_text(input_path);
    Parser parser(src);
    auto stmts = parser.parse_program();
    if(!parser.ok)
    {
        return 1;
    }

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    CodegenContext cg;
    cg.module = std::make_unique<llvm::Module>("mu_min", cg.ctx);

    llvm::Type* i8_ptr = llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(cg.ctx));
    std::vector<llvm::Type*> printf_params = {i8_ptr};
    llvm::FunctionType* printf_type = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(cg.ctx),
        printf_params,
        true
    );
    cg.printf_fn = llvm::Function::Create(
        printf_type,
        llvm::Function::ExternalLinkage,
        "printf",
        cg.module.get()
    );

    llvm::FunctionType* main_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(cg.ctx), false);
    cg.main_fn = llvm::Function::Create(
        main_type,
        llvm::Function::ExternalLinkage,
        "main",
        cg.module.get()
    );

    llvm::BasicBlock* entry = llvm::BasicBlock::Create(cg.ctx, "entry", cg.main_fn);
    cg.builder.SetInsertPoint(entry);
    cg.fmt_string = cg.builder.CreateGlobalStringPtr("%ld\n", "fmt");

    for(auto& stmt : stmts)
    {
        if(!stmt->codegen(cg))
        {
            return 1;
        }
    }

    cg.builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(cg.ctx), 0));
    if(llvm::verifyFunction(*cg.main_fn, &llvm::errs()))
    {
        return 1;
    }

    std::string object_path = output_path + ".o";
    if(!write_object_file(cg.module.get(), object_path))
    {
        return 1;
    }
    if(!link_executable(object_path, output_path))
    {
        std::cerr << "error: failed to link executable\n";
        return 1;
    }

    return 0;
}
