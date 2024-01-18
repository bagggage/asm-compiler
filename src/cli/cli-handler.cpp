#include "cli-handler.h"

#include <iostream>
#include <fstream>

#include "codegen/code-generator.h"
#include "syntax/parser.h"
#include "syntax/lexer.h"
#include "linking/linker.h"

using namespace ASM;
using namespace ASM::CLI;

const std::unordered_map<std::string, ArgKind> Argument::StrToKind = {
    { "o",          ArgKind::output },
    { "out",        ArgKind::output },
    { "i",          ArgKind::input },
    { "in",         ArgKind::output },
    { "show-ast",   ArgKind::show_ast },
    { "show-link",  ArgKind::show_linking },
    { "show-seg",   ArgKind::show_sections },
    { "show-sec",   ArgKind::show_sections },
    { "show-all",   ArgKind::show_all },
    { "s-ast",      ArgKind::show_ast },
    { "s-link",     ArgKind::show_linking },
    { "s-seg",      ArgKind::show_sections },
    { "s-sec",      ArgKind::show_sections },
    { "s-all",      ArgKind::show_all },
    { "l",          ArgKind::linking },
    { "f",          ArgKind::format },
    { "format",     ArgKind::format },
};

const std::unordered_map<std::string, CommandLineInterfaceHandler::Target> CommandLineInterfaceHandler::StrToTarget =
{
    { "bin", CliTarget::com },
    { "com", CliTarget::com },
    { "exe", CliTarget::exe },
    { "obj", CliTarget::object }
};

std::vector<Argument> CommandLineInterfaceHandler::ParseArguments(const char** argv, size_t argc)
{
    constexpr const char argChar = '-';
    std::vector<Argument> arguments;

    //Skip first argument, usually it is a launch directory
    for (unsigned int i = 1; i < argc; ++i) {
        if (argv[i][0] != argChar || Argument::StrToKind.count(&argv[i][1]) == 0)
        {
            std::cout << "Unknown argument \'" << argv[i] << "\' ignored" << std::endl;
            continue;
        }

        Argument arg(Argument::StrToKind.at(&argv[i][1]));

        if (arg.IsNeedValue())
        {
            if (i + 1 == argc || argv[i + 1][0] == argChar)
            {
                std::cout << "Value for argument \'" << argv[i] << "\' not provided" << std::endl;
                continue;
            }

            ++i;
            arg.SetValue(argv[i]);
        }

        arguments.push_back(arg);
    }

    return arguments;
}

CommandLineInterfaceHandler::CommandLineInterfaceHandler(int argc, const char** argv)
{
    auto arguments = ParseArguments(argv, argc); 

    for (auto& arg : arguments) {
        switch (arg.GetKind())
        {
        case ArgKind::output:
            config.outputFile = arg.GetValue();
            break;
        case ArgKind::input:
            config.inputFiles.push_back(arg.GetValue());
            break;
        case ArgKind::log_out:
            config.logOutput = arg.GetValue();
            break;
        case ArgKind::format:
        {
            if (StrToTarget.count(arg.GetValue()) == 0)
            {
                std::cout << "Unknown output format \'" << arg.GetValue() << "\', use help to see all supported formats" << std::endl;
                break;
            }

            auto oldTarget = config.target;
            config.target = StrToTarget.at(arg.GetValue());

            if (oldTarget == Target::linking_com)
                goto set_linking_format;
        }
            break;
        case ArgKind::show_all:
            config.debugInfo = 
            static_cast<uint8_t>(DebugInfo::symbol_table) | 
            static_cast<uint8_t>(DebugInfo::ast) |
            static_cast<uint8_t>(DebugInfo::sections) |
            static_cast<uint8_t>(DebugInfo::linking_targets);
            break;
        case ArgKind::show_ast:
            config.debugInfo |= static_cast<uint8_t>(DebugInfo::ast);
            break;
        case ArgKind::show_linking:
            config.debugInfo |= static_cast<uint8_t>(DebugInfo::linking_targets);
            break;
        case ArgKind::show_sym_table:
            config.debugInfo |= static_cast<uint8_t>(DebugInfo::symbol_table);
            break;
        case ArgKind::show_sections:
            break;
        case ArgKind::linking:
        set_linking_format:
            if (config.target == Target::com || config.target == Target::exe)
                *reinterpret_cast<uint8_t*>(&config.target) += static_cast<uint8_t>(Target::linking_com) - static_cast<uint8_t>(Target::com);
            else
                std::cout << "Incompatable output format with linking mode" << std::endl;
            break;
        default:
            assert(false);
            break;
        }
    }

    if (config.outputFile.empty() && config.inputFiles.empty() == false) {
        config.outputFile = config.inputFiles.back();
        config.outputFile.replace_extension("");
    }
}

std::string CommandLineInterfaceHandler::logDebugStringBuffer = std::string();

void CommandLineInterfaceHandler::PrintExpression(const AST::Expression* expression, bool inDepth, bool isLast, std::string& startString)
{
    auto& out = context->GetLogOutput();

    uint8_t temp = 0;

    if (inDepth)
        startString.push_back('\t');
    
    out << startString << (isLast ? '`' : '|') << '-';

    if (isLast == false)
        startString.push_back('|');

    if (expression->Is<AST::NumberExpr>())
        out << "\033[1mnum\033[0m \'" << expression->GetAs<AST::NumberExpr>()->value << '\'' << std::endl;
    else if (expression->Is<AST::LiteralExpr>())
        out << "\033[1mliteral\033[0m \'" << expression->GetAs<AST::LiteralExpr>()->value << '\'' << std::endl;
    else if (expression->Is<AST::RegisterExpr>()) {

        out << "\033[1mreg\033[0m \'" << (int)expression->GetAs<AST::RegisterExpr>()->GetIdentifier() << '\'' << std::endl;
    }
    else if (expression->Is<AST::SymbolExpr>())
    {
        const AST::SymbolExpr* symbolExpr = expression->GetAs<AST::SymbolExpr>();

        out << "\033[1msymbol\033[0m \'" << symbolExpr->GetName();

        if (context->GetSymbolTable().HasSymbol(symbolExpr->GetName()))
        {
            auto& symbol = context->GetSymbolTable().GetSymbol(symbolExpr->GetName());
    
            out << ": ";
    
            switch (symbol.GetDeclaration().GetScope())
            {
            case AST::SymbolDecl::Scope::Local: out << "local"; break;
            case AST::SymbolDecl::Scope::Extern: out << "extern"; break;
            case AST::SymbolDecl::Scope::Global: out << "global"; break;
            default:
                break;
            }
    
            out << '-' << (symbol.GetDeclaration().IsAddress() ? "address" : "literal");
        }

        out << '\'' << std::endl;
    }
    else if (expression->Is<AST::BinaryExpr>())
    {
        out << "\033[1mbinary\033[0m \'" << expression->GetAs<AST::BinaryExpr>()->operation << '\'' << std::endl;

        PrintExpression(expression->GetAs<AST::BinaryExpr>()->lhs.get(), true, false);
        PrintExpression(expression->GetAs<AST::BinaryExpr>()->rhs.get(), true);
    }
    else if (expression->Is<AST::UnaryExpr>())
    {
        out << "\033[1munary\033[0m \'" << expression->GetAs<AST::UnaryExpr>()->GetOperation() << '\'' << std::endl;

        PrintExpression(expression->GetAs<AST::UnaryExpr>()->GetExpression(), true);
    }
    else if (expression->Is<AST::MemoryExpr>())
    {
        out << "\033[1mmemory\033[0m" << std::endl;

        if (expression->GetAs<AST::MemoryExpr>()->GetSegOverride() != nullptr)
            PrintExpression(expression->GetAs<AST::MemoryExpr>()->GetSegOverride(), true, false);

        PrintExpression(expression->GetAs<AST::MemoryExpr>()->GetExpression(), true);
    }
    else if (expression->Is<AST::ParenExpr>())
    {
        out << "\033[1mparen\033[0m" << std::endl;

        PrintExpression(expression->GetAs<AST::ParenExpr>()->GetExpression(), true);
    }

    if (isLast == false)
        startString.pop_back();
    if (inDepth)
        startString.pop_back();
}

void CommandLineInterfaceHandler::PrintSymbolDecl(const AST::Declaration* ptr)
{
    auto& out = context->GetLogOutput();

    if (ptr->Is<AST::ConstantDecl>())
    {
        out << "\033[1mConstant\033[0m: " << ptr->GetAs<AST::ConstantDecl>()->GetName() << std::endl;
        PrintExpression(&ptr->GetAs<AST::ConstantDecl>()->GetExpression(), true);
    }
    else if (ptr->Is<AST::SectionDecl>())
    {
        out << "\033[1mSection\033[0m: " << ptr->GetAs<AST::SectionDecl>()->GetName() << std::endl;
    }
    else if (ptr->Is<AST::LableDecl>())
    {
        out << "\033[1mLable\033[0m: " << ptr->GetAs<AST::LableDecl>()->GetName() << std::endl;
    }
}

void CommandLineInterfaceHandler::LogAST(AbstractSyntaxTree& ast)
{
    auto& out = context->GetLogOutput();

    out << "[AST Interpretation]:" << std::endl;

    for (auto& ptr : ast)
    {
        out << '>';

        if (ptr->Is<AST::SymbolDecl>())
        {
            PrintSymbolDecl(ptr->GetAs<AST::SymbolDecl>());
        }
        else if (ptr->Is<AST::InstructionStmt>())
        {
            out << "\033[1mInstruction\033[0m: " << ptr->GetAs<AST::InstructionStmt>()->GetMnemonic() << std::endl;
            
            auto& operands = ptr->GetAs<AST::InstructionStmt>()->GetOperands();

            for (auto& operand : operands)
                PrintExpression(operand.get(), true, &operand == &operands.back());
        }
        else if (ptr->Is<AST::DefineDataStmt>())
        {
            out << "\033[1mData define\033[0m: " << (int)ptr->GetAs<AST::DefineDataStmt>()->GetDataUnitSize() << "b" << std::endl;

            auto& operands = ptr->GetAs<AST::DefineDataStmt>()->GetUnits();

            for (auto& operand : operands)
                PrintExpression(operand.get(), true, &operand == &operands.back());
        }
        else if (ptr->Is<AST::AlignStmt>())
        {
            out << "\033[1mAlign\033[0m: " << std::endl;
            PrintExpression(ptr->GetAs<AST::AlignStmt>()->GetValueExpression(), true);
        }
        else if (ptr->Is<AST::OffsetStmt>())
        {
            out << "\033[1mOffset\033[0m: " << std::endl;
            PrintExpression(ptr->GetAs<AST::OffsetStmt>()->GetValueExpression(), true);   
        }
        else if (ptr->Is<AST::OrgStmt>())
        {
            out << "\033[1mOrigin\033[0m: " << std::endl;
            PrintExpression(ptr->GetAs<AST::OrgStmt>()->GetValueExpression(), true);
        }
        else if (ptr->Is<AST::StackStmt>())
        {
            out << "\033[1mStack\033[0m: " << std::endl;
            PrintExpression(ptr->GetAs<AST::StackStmt>()->GetValueExpression(), true);
        }
    }

    out << std::endl;
}

void CommandLineInterfaceHandler::LogSection(ASM::Section& section)
{
    auto& out = context->GetLogOutput();

    out << "Section: " << section.GetName() << std::endl;

    if (config.debugInfo & static_cast<uint8_t>(DebugInfo::linking_targets))
    {
        out << "[Linking Targets]:" << std::endl;

        for (auto& link : section.GetLinkingTargets())
        {
            out << "0x" << std::setfill('0') << std::setw(16) << link.GetSectionOffset()
            << ": " << (link.GetKind() == ASM::LinkingTarget::Kind::RelativeAddress ? "Relative" : "Value\\Absolute")
            << ' ' << (link.GetType() == ASM::LinkingTarget::Type::Integer ? "int" : "float")
            << std::dec << static_cast<uint16_t>(link.GetSize()) * 8 << "_t" << ':' << std::endl;

            PrintExpression(link.GetExpression());
        }
    }

    out << std::endl;
}

void CommandLineInterfaceHandler::LogLinkingTarget(ASM::LinkingTarget& link)
{
    auto& out = context->GetLogOutput();

    out << "0x" << std::setfill('0') << std::setw(16) << link.GetSectionOffset()
    << ": " << (link.GetKind() == ASM::LinkingTarget::Kind::RelativeAddress ? "Relative" : "Value\\Absolute")
    << ' ' << (link.GetType() == ASM::LinkingTarget::Type::Integer ? "int" : "float")
    << std::dec << static_cast<uint16_t>(link.GetSize()) * 8 << "_t" << ':' << std::endl;

    PrintExpression(link.GetExpression());
}

void CommandLineInterfaceHandler::LogSymbolTable(ASM::SymbolTable& symbolTable)
{
    auto& out = context->GetLogOutput();

    out << "[Symbol Table]:" << std::endl;

    for (auto& pair : symbolTable.GetSymbolsMap())
    {
        out << "Symbol \'" << pair.first << "\':" << std::endl;
        PrintSymbolDecl(&pair.second.GetDeclaration());
        
        if (pair.second.IsEvaluated())
            out << "Value: " << pair.second.GetValue().GetAsInt() << std::endl;
    }
}

bool CommandLineInterfaceHandler::Handle()
{
    if (config.inputFiles.size() == 0)
    {
        std::cout << "No input files, use \'-i\' argument to provie file path" << std::endl;
        return false;
    }

    std::ifstream in(config.inputFiles.back());
    std::ofstream out(config.outputFile);
    std::ofstream logOutput;

    if (in.is_open() == false)
    {
        std::cout << "Can't open input file \'" << config.inputFiles.back() << "\'" << std::endl;
        return false;
    }

    if (out.is_open() == false)
    {
        std::cout << "Can't open output file \'" << config.outputFile << "\'" << std::endl;
        return false;
    }

    context = std::make_unique<AssemblyContext>(in, Arch::Arch8086::InstructionSet);

    if (config.logOutput.empty() == false)
    {
        logOutput.open(config.logOutput);

        if (logOutput.is_open() == false)
        {
            std::cout << "Can't open log output file \'" << config.logOutput << "\'" << std::endl;
            return false;
        }

        context->SetLogOutput(logOutput);
    }

    Lexer lexer(*context);
    Parser parser(*context);
    Codegen::CodeGenerator codeGenerator(*context);
    Linker linker(*context);

    {
        ASM::Token token;
    
        while (lexer.GetNextToken(token) || token.Is(ASM::TokKind::eof) == false)
            parser.PushToken(std::move(token));
    
        parser.PushToken(std::move(token));
    }

    AbstractSyntaxTree ast = parser.Parse();

    if (config.debugInfo & static_cast<uint8_t>(DebugInfo::ast))
        LogAST(ast);

    codeGenerator.ProccessAST(ast);
    
    std::unique_ptr<AssembledObject> assembledObject;
    
    switch (config.target)
    {
    case Target::com:
        assembledObject = linker.Link(LinkingFormat::RawBinary);
        break;
    case Target::exe:
        assembledObject = linker.Link(LinkingFormat::DosExecutable);
        break;
    default:
        break;
    }
    
    if (context->HasErrors())
    {
        const std::string result = "Build failed: " + std::to_string(context->GetErrorsCount()) + " errors";
        context->Info(result.c_str());
        return false;
    }

    if (config.debugInfo & static_cast<uint8_t>(DebugInfo::sections))
    {
        for (auto& pair : context->GetTranslationUnit().GetSectionMap())
            LogSection(pair.second);
    }
    if (config.debugInfo & static_cast<uint8_t>(DebugInfo::symbol_table))
        LogSymbolTable(context->GetSymbolTable());

    if (assembledObject->Serialize(out) == false)
    {
        context->Error("Can't write assembled object to file, something went wrong...");
        return false;
    }

    return true;
}
