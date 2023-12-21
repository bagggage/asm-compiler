#ifndef __CLI_HANDLER_H
#define __CLI_HANDLER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

#include "context/context.h"

namespace ASM::CLI
{
    class Argument
    {
    public:
        enum class Kind : uint8_t
        {
            output,
            input,
            format,
            log_out,
            show_ast,
            show_sections,
            show_linking,
            show_sym_table,
            show_all,
            linking
        };

        static const std::unordered_map<std::string, Kind> StrToKind;
    private:
        Kind kind;
        std::string value;
    public:
        Argument(Kind kind) : kind(kind) {}

        inline void SetValue(const std::string& value) { this->value = value; }

        inline Kind GetKind() const { return kind; }
        inline const std::string& GetValue() const { return value; }

        inline bool IsNeedValue() const { return kind >= Kind::output && kind <= Kind::log_out; }
    };

    using ArgKind = Argument::Kind;

    class CommandLineInterfaceHandler
    {
    private:
        enum class Target : uint8_t
        {
            com,
            exe,
            object,
            linking_com,
            linking_exe
        };

        enum class DebugInfo : uint8_t
        {
            none = 0b0000,
            ast = 0b0001,
            linking_targets = 0b0010,
            symbol_table = 0b0100,
            sections = 0b1000
        };

        using CliTarget = CommandLineInterfaceHandler::Target;

        struct Config
        {
            Target target = Target::com;
            uint8_t debugInfo = static_cast<uint8_t>(DebugInfo::none);

            std::vector<std::filesystem::path> inputFiles;
            std::filesystem::path outputFile;
            std::filesystem::path logOutput;
        };

        static const std::unordered_map<std::string, Target> StrToTarget;

        Config config;
        std::unique_ptr<ASM::AssemblyContext> context;

        static std::vector<Argument> ParseArguments(const char** argv, size_t argc);
    public:
        CommandLineInterfaceHandler(int argc, const char** argv);

        bool Handle();
    };
}

#endif