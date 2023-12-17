#include "cli-handler.h"

#include <iostream>
#include <fstream>

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

CommandLineInterfaceHandler::CommandLineInterfaceHandler(int argc, const char** argv)
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

    for (auto arg : arguments) {
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
            if (StrToTarget.count(arg.GetValue()) == 0)
            {
                std::cout << "Unknown output format \'" << arg.GetValue() << "\', use help to see all supported formats" << std::endl;
                break;
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
}

bool CommandLineInterfaceHandler::Handle()
{
    if (config.inputFiles.size() == 0)
    {
        std::cout << "No input files, use \'-i\' argument to provie file path" << std::endl;
        return false;
    }

    std::ifstream file(config.inputFiles.back());

    if (file.is_open() == false)
    {
        std::cout << "Can't open input file \'" << config.inputFiles.back() << "\'" << std::endl;
    }

    //context = std::make_unique<AssemblyContext>();
}
