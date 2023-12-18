#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <future>
#include <thread>

#include "cli/cli-handler.h"

#include "syntax/expressions.h"
#include "syntax/lexer.h"
#include "syntax/parser.h"

#include "codegen/code-generator.h"
#include "linking/linker.h"

#ifdef DEBUG

#define TT_TO_STR(x) { ASM::TokKind::x, #x }

std::unordered_map<ASM::TokKind, const char*> TokKindToString = 
{
    TT_TO_STR(eof),
    TT_TO_STR(identifier),
    TT_TO_STR(string_literal),
    TT_TO_STR(char_constant),
    TT_TO_STR(num_constant),
    TT_TO_STR(reg),

    TT_TO_STR(kw_global),
    TT_TO_STR(kw_extern),
    TT_TO_STR(kw_org),
    TT_TO_STR(kw_section),
    TT_TO_STR(kw_segment),
    TT_TO_STR(kw_offset),
    TT_TO_STR(kw_align),
    TT_TO_STR(kw_dup),

    TT_TO_STR(l_square),
    TT_TO_STR(r_square),
    TT_TO_STR(l_paren),
    TT_TO_STR(r_paren),
    TT_TO_STR(comma),
    TT_TO_STR(colon),

    TT_TO_STR(plus),
    TT_TO_STR(minus),
    TT_TO_STR(slash),
    TT_TO_STR(star),
    TT_TO_STR(caret),
    TT_TO_STR(pipe),
    TT_TO_STR(amp),
    TT_TO_STR(lessless),
    TT_TO_STR(greatgreat),

    TT_TO_STR(question),
    TT_TO_STR(at),
    TT_TO_STR(dolar),
    TT_TO_STR(dolardolar),

    TT_TO_STR(unknown)
};

void Print(ASM::Token& token)
{
    std::cout << "Tok (line " << token.GetLocation().line + 1 << "): " << TokKindToString.at(token.GetKind());

    if (token.Is(ASM::TokKind::char_constant))
        std::cout << "\t|\t\'"  << token.GetAsNum()->GetAscii() << '\'';
    else if (token.Is(ASM::TokKind::num_constant))
        std::cout << "\t|\t" << token.GetAsNum()->GetNum();
    else if (token.Is(ASM::TokKind::reg))
    {
        for (auto& pair : ASM::Lexer::IdentifierToRegId)
            if (pair.second == token.GetAsNum()->GetRegId())
            {
                std::cout << "\t|\t" << pair.first;

                break;
            }
    }
    else if (token.Is(ASM::TokKind::string_literal))
        std::cout << "\t|\t\"" << token.GetAsString()->GetValue() << '\"';
    else if (token.Is(ASM::TokKind::identifier))
        std::cout << "\t|\t" << token.GetAsString()->GetValue();

    std::cout << std::endl;
}

#endif

static std::string stringBuffer = std::string();
static ASM::AssemblyContext* contextPtr = nullptr;

void printExpression(ASM::AST::Expression* expression, bool inDepth = false, bool isLast = true, std::string& startString = stringBuffer)
{
    uint8_t temp = 0;

    if (inDepth)
        startString.push_back('\t');
    
    std::cout << startString << (isLast ? '`' : '|') << '-';

    if (isLast == false)
        startString.push_back('|');

    if (expression->Is<ASM::AST::NumberExpr>())
        std::cout << "\033[1mnum\033[0m \'" << reinterpret_cast<ASM::AST::NumberExpr*>(expression)->value << '\'' << std::endl;
    else if (expression->Is<ASM::AST::LiteralExpr>())
        std::cout << "\033[1mliteral\033[0m \'" << reinterpret_cast<ASM::AST::LiteralExpr*>(expression)->value << '\'' << std::endl;
    else if (expression->Is<ASM::AST::RegisterExpr>())
        std::cout << "\033[1mreg\033[0m \'" << (int)reinterpret_cast<ASM::AST::RegisterExpr*>(expression)->GetIdentifier() << '\'' << std::endl;
    else if (expression->Is<ASM::AST::SymbolExpr>())
    {
        ASM::AST::SymbolExpr* symbolExpr = expression->GetAs<ASM::AST::SymbolExpr>();

        std::cout << "\033[1msymbol\033[0m \'" << symbolExpr->GetName();

        if (contextPtr->GetSymbolTable().HasSymbol(symbolExpr->GetName()))
        {
            auto& symbol = contextPtr->GetSymbolTable().GetSymbol(symbolExpr->GetName());
    
            std::cout << ": ";
    
            switch (symbol.GetDeclaration().GetScope())
            {
            case ASM::AST::SymbolDecl::Scope::Local: std::cout << "local"; break;
            case ASM::AST::SymbolDecl::Scope::Extern: std::cout << "extern"; break;
            case ASM::AST::SymbolDecl::Scope::Global: std::cout << "global"; break;
            default:
                break;
            }
    
            std::cout << '-' << (symbol.GetDeclaration().IsAddress() ? "address" : "literal");
        }

        std::cout << '\'' << std::endl;
    }
    else if (expression->Is<ASM::AST::BinaryExpr>())
    {
        std::cout << "\033[1mbinary\033[0m \'" << reinterpret_cast<ASM::AST::BinaryExpr*>(expression)->operation << '\'' << std::endl;

        printExpression(reinterpret_cast<ASM::AST::BinaryExpr*>(expression)->lhs.get(), true, false);
        printExpression(reinterpret_cast<ASM::AST::BinaryExpr*>(expression)->rhs.get(), true);
    }
    else if (expression->Is<ASM::AST::UnaryExpr>())
    {
        std::cout << "\033[1munary\033[0m \'" << reinterpret_cast<ASM::AST::UnaryExpr*>(expression)->GetOperation() << '\'' << std::endl;

        printExpression(reinterpret_cast<ASM::AST::UnaryExpr*>(expression)->GetExpression(), true);
    }
    else if (expression->Is<ASM::AST::MemoryExpr>())
    {
        std::cout << "\033[1mmemory\033[0m" << std::endl;

        if (expression->GetAs<ASM::AST::MemoryExpr>()->GetSegOverride() != nullptr)
            printExpression(reinterpret_cast<ASM::AST::MemoryExpr*>(expression)->GetSegOverride(), true, false);

        printExpression(reinterpret_cast<ASM::AST::MemoryExpr*>(expression)->GetExpression(), true);
    }
    else if (expression->Is<ASM::AST::ParenExpr>())
    {
        std::cout << "\033[1mparen\033[0m" << std::endl;

        printExpression(reinterpret_cast<ASM::AST::ParenExpr*>(expression)->GetExpression(), true);
    }

    if (isLast == false)
        startString.pop_back();
    if (inDepth)
        startString.pop_back();
}

int main(int argc, const char** argv)
{
    //ASM::CLI::CommandLineInterfaceHandler cliHandler(argc, argv);

    std::ifstream file;

    if (argc > 1)
        file.open(argv[1]);
    else
        file.open("source.asm");

    if (file.is_open() == false)
        return 0;

    ASM::AssemblyContext context(file, ASM::Arch::Arch8086::InstructionSet);
    contextPtr = &context;

    file.close();

    ASM::Lexer lexer(context);
    ASM::Parser parser(context);

    context.SetParallelMode(lexer);

    auto begin = std::chrono::system_clock::now();

    auto future = std::async(std::launch::async, [&]() 
    {
        ASM::Token token;
    
        while (lexer.GetNextToken(token) || token.Is(ASM::TokKind::eof) == false)
            parser.PushToken(std::move(token));
    
        parser.PushToken(std::move(token));
    });

    future.wait();

    ASM::AbstractSyntaxTree tree = parser.Parse();

    ASM::Codegen::CodeGenerator codeGenerator(context);

    ASM::TranslationUnit& unit = codeGenerator.ProccessAST(tree);
    ASM::Linker linker(context);
    auto assembledObject = linker.Link(ASM::LinkingFormat::RawBinary);

    ASM::Codegen::MachineCode& code = reinterpret_cast<ASM::RawBinary*>(assembledObject.get())->GetCode();//unit.GetSectionMap().begin()->second.GetCode();

    std::ofstream out("test.com");
    assembledObject->Serialize(out);
    out.close();
    
    for (auto hex : code.code)
        std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<uint16_t>(hex) << ' ';

    std::cout << std::endl;

    std::cout << "Total time: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - begin).count() / 1000.f << "ms" << std::endl;

    unsigned int errorsCount = 0;

    while (context.HasMessages())
    {
        auto msg = context.GetMessage();

        if (msg->Is(ASM::Message::Kind::Error))
        {
            std::cerr << msg->What() << std::endl;
            ++errorsCount;
        }
        else
        {
            std::cout << msg->What() << std::endl;
        }
    }

    if (errorsCount != 0)
    {
        std::cout << "\033[1;31m[Build failed]\033[1;0m Number of errors:\033[0m " << errorsCount << std::endl; 

        return 0;
    }

    std::cout << std::endl; 

    return 0;

    for (auto& section : unit.GetSectionMap())
    {
        std::cout << "Section: " << section.first << std::endl;
        std::cout << "[Linking Targets]:" << std::endl;

        for (auto& link : section.second.GetLinkingTargets())
        {
            std::cout << "0x" << std::setfill('0') << std::setw(16) << link.GetSectionOffset()
            << ": " << (link.GetKind() == ASM::LinkingTarget::Kind::RelativeAddress ? "Relative" : "Value\\Absolute")
            << ' ' << (link.GetType() == ASM::LinkingTarget::Type::Integer ? "int" : "float")
            << std::dec << static_cast<uint16_t>(link.GetSize()) * 8 << "_t" << ':' << std::endl;

            printExpression(link.GetExpression());
        }
    }

    std::cout << std::endl;

    //return 0;

    std::cout << "[AST Interpretation]:" << std::endl;

    for (auto& ptr : tree)
    {
        std::cout << '>';

        if (ptr->Is<ASM::AST::ConstantDecl>())
        {
            std::cout << "\033[1mConstant\033[0m: " << reinterpret_cast<ASM::AST::ConstantDecl*>(ptr.get())->GetName() << std::endl;

            printExpression(&reinterpret_cast<ASM::AST::ConstantDecl*>(ptr.get())->GetExpression(), true);
        }
        else if (ptr->Is<ASM::AST::SectionDecl>())
        {
            std::cout << "\033[1mSegment\033[0m: " << reinterpret_cast<ASM::AST::SectionDecl*>(ptr.get())->GetName() << std::endl;
        }
        else if (ptr->Is<ASM::AST::InstructionStmt>())
        {
            std::cout << "\033[1mInstruction\033[0m: " << reinterpret_cast<ASM::AST::InstructionStmt*>(ptr.get())->GetMnemonic() << std::endl;
            
            auto& operands = reinterpret_cast<ASM::AST::InstructionStmt*>(ptr.get())->GetOperands();

            for (auto& operand : operands)
                printExpression(operand.get(), true, &operand == &operands.back());
        }
        else if (ptr->Is<ASM::AST::LableDecl>())
        {
            std::cout << "\033[1mLable\033[0m: " << reinterpret_cast<ASM::AST::LableDecl*>(ptr.get())->GetName() << std::endl;
        }
        else if (ptr->Is<ASM::AST::DefineDataStmt>())
        {
            std::cout << "\033[1mData define\033[0m: " << (int)reinterpret_cast<ASM::AST::DefineDataStmt*>(ptr.get())->GetDataUnitSize() << "b" << std::endl;

            auto& operands = reinterpret_cast<ASM::AST::DefineDataStmt*>(ptr.get())->GetUnits();

            for (auto& operand : operands)
                printExpression(operand.get(), true, &operand == &operands.back());
        }
    }

    return 0;
}