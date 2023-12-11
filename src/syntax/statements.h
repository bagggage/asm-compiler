#ifndef __AST_STATEMENTS_H
#define __AST_STATEMENTS_H

#include "node.h"
#include "expressions.h"
#include "codegen/machine-code.h"
#include "arch/arch.h"

namespace ASM
{
    class Parser;
    class AssemblyContext;

    namespace Codegen
    {
        class CodeGenerator;
    }
}

namespace ASM::AST
{
    struct Statement : public Node
    {
    public:
        virtual Codegen::MachineCode CodeGen(Codegen::CodeGenerator& generator) const = 0;
    };

    struct InstructionStmt : public Statement
    {
    private:
        static constexpr uint8_t minModRm16DisplacementSize = 1;
        static constexpr uint8_t maxModRm16DisplacementSize = 2;

        std::string mnemonic;
        std::vector<std::unique_ptr<Expression>> operands;

        friend class ASM::Parser;

        static bool IsCorrectMemoryExprPass(AST::Expression* expression, bool registersCheck = false);
        static bool IsCorrectMemoryExpr(AST::MemoryExpr* expression);

        static void EncodeModRM
        (
            Arch::ModRM source, const std::unique_ptr<Expression>& operand,
            Codegen::CodeGenerator& generator, Codegen::MachineCode& code
        );

        static void EncodeImm
        (
            Expression* operand,
            Arch::Operand operandPrototype, Codegen::CodeGenerator& generator,
            Codegen::MachineCode& code
        );
    public:
        using Operands_t = decltype(InstructionStmt::operands);

        inline const std::string& GetMnemonic() const { return mnemonic; }
        inline std::vector<std::unique_ptr<Expression>>& GetOperands() { return operands; }

        Codegen::MachineCode CodeGen(Codegen::CodeGenerator& generator) const override;
    };

    struct DuplicateStmt : public virtual Statement
    {
    private:
        friend class ASM::Parser;

        std::unique_ptr<Expression> countExpression;
        std::unique_ptr<Expression> valueExpression;        
    public:
        //bool IsDependent() const override;
        //bool Simplify() override;

        Codegen::MachineCode CodeGen(Codegen::CodeGenerator& generator) const override;
    };

    struct DefineDataStmt : public Statement
    {
    private:
        friend class ASM::Parser;

        uint8_t dataUnitSize = 1;

        std::vector<std::unique_ptr<Expression>> units;
    public:
        inline uint8_t GetDataUnitSize() const { return dataUnitSize; }
        inline std::vector<std::unique_ptr<Expression>>& GetUnits() { return units; }

        Codegen::MachineCode CodeGen(Codegen::CodeGenerator& generator) const override;
    };
}

#endif