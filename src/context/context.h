#ifndef __CONTEXT_H
#define __CONTEXT_H

#include <cassert>
#include <string>
#include <iostream>
#include <queue>
#include <memory>
#include <unordered_map>

#include "message.h"
#include "symbol-table.h"
#include "syntax/declarations.h"
#include "translation-unit.h"
#include "arch/arch.h"

namespace ASM
{
    class Lexer;

    enum class AssemblyMode : uint8_t
    {
        Direct,
        Parallel
    };

    class AssemblyContext
    {
    public:
        using InstructionSet_t = std::unordered_map<std::string, std::vector<Arch::Instruction>>;
    private:
        std::string currentSource;

        const InstructionSet_t* instructionSet = nullptr; 

        TranslationUnit translationUnit;
        SymbolTable symbolTable;

        std::ostream* logStream = &std::cout;
        std::queue<std::unique_ptr<Message>> messageQueue;

        size_t errorsCount = 0;

        Message* lastError = nullptr;

        AssemblyMode mode = AssemblyMode::Direct;

        static Message* MakeMessage(Message::Kind kind, const char* message, SourceLocation location, size_t length);
    public:
        AssemblyContext(std::string&& source, const InstructionSet_t& instructionSet) :
            currentSource(source), instructionSet(&instructionSet) {}

        AssemblyContext(const std::string& source, const InstructionSet_t& instructionSet) :
            currentSource(source), instructionSet(&instructionSet) {}

        AssemblyContext(std::istream& sourceStream, const InstructionSet_t& instructionSet);

        static constexpr std::string_view UnnamedSection = "$unnamed";

        inline void SetDirectMode() { mode = AssemblyMode::Direct; };
        inline void SetParallelMode(Lexer& lexer) { mode = AssemblyMode::Parallel; }

        inline SymbolTable& GetSymbolTable() { return symbolTable; }
        inline const SymbolTable& GetSymbolTable() const { return symbolTable; }

        inline void SetLogOutput(std::ostream& stream) { logStream = &stream; }
        inline void SetInstructionSet(const InstructionSet_t& set) { instructionSet = &set; }

        inline bool IsCurrentMode(AssemblyMode intendentMode) const { return (mode == intendentMode); }

        inline std::ostream& GetLogOutput() const { return *logStream; }
        inline const std::string& GetSource() const { return currentSource; }
        inline bool HasMessages() const { return (messageQueue.empty() == false); }

        Message* GetLastError();
        std::unique_ptr<Message> GetMessage();

        inline const InstructionSet_t& GetInstructionSet() const { return *instructionSet; };
        inline TranslationUnit& GetTranslationUnit() { return translationUnit; }

        void Info(const char* message, SourceLocation location = SourceLocation(), size_t length = 0);
        void Warn(const char* message, SourceLocation location = SourceLocation(), size_t length = 0);
        void Error(const char* message, SourceLocation location = SourceLocation(), size_t length = 0);

        inline bool HasErrors() const { return errorsCount > 0; }
        inline size_t GetErrorsCount() const { return errorsCount; }
    };
}

#endif