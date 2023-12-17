#ifndef __ASM_CODE_GENERATOR_H
#define __ASM_CODE_GENERATOR_H

#include <optional>

#include "context/context.h"
#include "syntax/ast.h"
#include "context/translation-unit.h"

#include "utils/small-vector.h"

namespace ASM::Codegen
{
    class CodeGenerator
    {
    private:
        AssemblyContext* context = nullptr;

        MachineCode* currentSectionCode = nullptr;
        Section* currentSection = nullptr;

        void ChangeCurrentSection(const std::string& sectionName);

        bool ResolveExpressionDependencies(
            const AST::Expression* expression,
            const std::string& symbolName,
            std::unordered_map<std::string, int64_t>& symbolMap
        ) const;

        static constexpr uint8_t highPriority = 2;
        static constexpr uint8_t lowPriority = 1;
    public:
        CodeGenerator(AssemblyContext& context) : context(&context) {}

        static uint8_t EvaluateLiteralByteSize(int64_t value);
        static uint8_t GetOperandTypePriority(Arch::OpType type);
        static Arch::OpType GetDirectEncodingOfRegisterOpType(AST::RegisterExpr* registerExpression);
        uint8_t EvaluateDependentOperandSize(const AST::Expression* operand) const;
        SmallVector<Arch::OperandEvaluation, 4> EvaluateOperands(const AST::InstructionStmt::Operands_t& operands) const;

        inline static uint8_t GetNopInstructionOpcode() { return Arch::Arch8086::InstructionSet.at("NOP").back().opcode.back(); }

        inline AssemblyContext& GetContext() const { return *context; }

        const Arch::Instruction* ChooseInstructionByOperands(const std::string& mnemonic, const AST::InstructionStmt::Operands_t& operands) const;
        bool IsExpressionHasAddressSymbol(AST::Expression* expression) const;

        void MakeAbsoluteLinkTarget(AST::Expression* expression, uint8_t offset, uint8_t size);
        void MakeRelativeLinkTarget(AST::Expression* expression, uint8_t offset, uint8_t size, uint8_t relativeOrigin);
        void MakeValueLinkTarget(AST::Expression* expression, uint8_t offset, uint8_t size, LinkingTarget::Type type);

        std::optional<int64_t> ResolveExpression(const AST::Expression* expression) const;

        inline const MachineCode& GetCurrentSectionCode() const { return *currentSectionCode; }

        TranslationUnit& ProccessAST(AbstractSyntaxTree& ast);
    };
}

#endif