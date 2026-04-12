#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#include "lexer.hpp"
#include "sema.hpp"

static void printTokens(const std::string& source, std::ostream& out)
{
    mu::Lexer lexer(source);
    out << "tokens:\n";
    for(;;)
    {
        mu::Token token = lexer.next();
        out << "  " << token.toString() << '\n';
        if(token.kind == mu::TokenKind::EOF_TOKEN)
        {
            break;
        }
    }
}

int main(int argc, char** argv)
{
    std::string source;
    if(argc > 1)
    {
        std::ifstream input(argv[1]);
        if(!input)
        {
            std::cerr << "failed to open source file: " << argv[1] << "\n";
            return 1;
        }
        std::ostringstream buffer;
        buffer << input.rdbuf();
        source = buffer.str();
    }
    else
    {
        source = R"(
            age := 21;
            years := 21;
            years = 22;
            count: i32 = 21;
        )";
    }

    printTokens(source, std::cout);

    MiniSemanticChecker checker(std::move(source));
    const auto&         diagnostics = checker.run();

    if(diagnostics.empty())
    {
        std::cout << "semantic check passed\n";
        return 0;
    }

    for(const auto& diagnostic : diagnostics)
    {
        std::cerr << diagnostic.where.line << ':' << diagnostic.where.column << ": error: " << diagnostic.message << "\n";
    }

    return 1;
}
